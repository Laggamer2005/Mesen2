#include "pch.h"
#include "Debugger/Debugger.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/CdlManager.h"
#include "Debugger/Disassembler.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"
#include "Shared/MessageManager.h"
#include "Utilities/VirtualFile.h"
#include "Utilities/FolderUtilities.h"

CodeDataLogger::CodeDataLogger(Debugger* debugger, MemoryType memType, uint32_t memSize, CpuType cpuType, uint32_t romCrc32)
    : _memType(memType), _cpuType(cpuType), _memSize(memSize), _romCrc32(romCrc32), _cdlData(new uint8_t[memSize])
{
    Reset();
    debugger->GetCdlManager()->RegisterCdl(memType, this);
}

CodeDataLogger::~CodeDataLogger()
{
    delete[] _cdlData;
}

void CodeDataLogger::Reset()
{
    memset(_cdlData, 0, _memSize);
}

uint8_t* CodeDataLogger::GetRawData()
{
    return _cdlData;
}

uint32_t CodeDataLogger::GetSize()
{
    return _memSize;
}

MemoryType CodeDataLogger::GetMemoryType()
{
    return _memType;
}

bool CodeDataLogger::LoadCdlFile(const string& cdlFilepath, bool autoResetCdl)
{
    VirtualFile cdlFile(cdlFilepath);
    if (cdlFile.IsValid()) {
        vector<uint8_t> cdlData;
        cdlFile.ReadFile(cdlData);

        if (cdlData.size() >= _memSize + HeaderSize) {
            Reset();

            if (memcmp(cdlData.data(), "CDLv2", 5) == 0) {
                uint32_t savedCrc = cdlData[5] | (cdlData[6] << 8) | (cdlData[7] << 16) | (cdlData[8] << 24);
                if ((!autoResetCdl || savedCrc == _romCrc32)) {
                    memcpy(_cdlData, cdlData.data() + HeaderSize, _memSize);
                    InternalLoadCdlFile(cdlData.data() + HeaderSize, (uint32_t)cdlData.size() - HeaderSize);
                }
            } else {
                MessageManager::Log("[Warning] CDL file doesn't contain header/CRC and may be incompatible.");

                // Older CRC-less CDL file, use as-is without checking CRC to avoid data loss
                memcpy(_cdlData, cdlData.data(), _memSize);
                InternalLoadCdlFile(cdlData.data(), (uint32_t)cdlData.size());
            }

            return true;
        }
    }
    return false;
}

bool CodeDataLogger::SaveCdlFile(const string& cdlFilepath)
{
    ofstream cdlFile(cdlFilepath, ios::out | ios::binary);
    if (cdlFile) {
        cdlFile.write("CDLv2", 5);
        cdlFile.write((char*)&_romCrc32, 4);
        cdlFile.write((char*)_cdlData, _memSize);
        InternalSaveCdlFile(cdlFile);
        cdlFile.close();
        return true;
    }
    return false;
}

string CodeDataLogger::GetCdlFilePath(const string& romName)
{
    return FolderUtilities::CombinePath(FolderUtilities::GetDebuggerFolder(), FolderUtilities::GetFilename(romName, false) + ".cdl");
}

CdlStatistics CodeDataLogger::GetStatistics()
{
    uint32_t codeSize = 0;
    uint32_t dataSize = 0;

    for (uint32_t i = 0; i < _memSize; i++) {
        if (IsCode(i)) {
            codeSize++;
        } else if (IsData(i)) {
            dataSize++;
        }
    }

    CdlStatistics stats = {};
    stats.CodeBytes = codeSize;
    stats.DataBytes = dataSize;
    stats.TotalBytes = _memSize;
    return stats;
}

bool CodeDataLogger::IsCode(uint32_t absoluteAddr)
{
    return (_cdlData[absoluteAddr] & CdlFlags::Code) != 0;
}

bool CodeDataLogger::IsJumpTarget(uint32_t absoluteAddr)
{
    return (_cdlData[absoluteAddr] & CdlFlags::JumpTarget) != 0;
}

bool CodeDataLogger::IsSubEntryPoint(uint32_t absoluteAddr)
{
    return (_cdlData[absoluteAddr] & CdlFlags::SubEntryPoint) != 0;
}

bool CodeDataLogger::IsData(uint32_t absoluteAddr)
{
    return (_cdlData[absoluteAddr] & CdlFlags::Data) != 0;
}

void CodeDataLogger::SetCdlData(uint8_t* cdlData, uint32_t length)
{
    memcpy(_cdlData, cdlData, std::min(length, _memSize));
}

void CodeDataLogger::GetCdlData(uint32_t offset, uint32_t length, uint8_t* cdlData)
{
    memcpy(cdlData, _cdlData + offset, std::min(length, _memSize - offset));
}

uint8_t CodeDataLogger::GetFlags(uint32_t addr)
{
    return _cdlData[addr];
}

uint32_t CodeDataLogger::GetFunctions(uint32_t functions[], uint32_t maxSize)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < _memSize; i++) {
        if (IsSubEntryPoint(i)) {
            functions[count] = i;
            count++;

            if (count == maxSize) {
                break;
            }
        }
    }
    return count;
}

void CodeDataLogger::MarkBytesAs(uint32_t start, uint32_t end, uint8_t flags)
{
    for (uint32_t i = start; i <= end; i++) {
        _cdlData[i] = (_cdlData[i] & 0xFC) | flags;
    }
}

void CodeDataLogger::StripData(uint8_t* romBuffer, CdlStripOption flag)
{
    switch (flag) {
        case CdlStripOption::StripUnused:
            for (uint32_t i = 0; i < _memSize; i++) {
                if (_cdlData[i] == 0) {
                    romBuffer[i] = 0;
                }
            }
            break;
        case CdlStripOption::StripUsed:
            for (uint32_t i = 0; i < _memSize; i++) {
                if (_cdlData[i] != 0) {
                    romBuffer[i] = 0;
                }
            }
            break;
    }
}

void CodeDataLogger::RebuildPrgCache(Disassembler* dis)
{
    AddressInfo addrInfo;
    addrInfo.Type = _memType;
    for (uint32_t i = 0; i < _memSize; i++) {
        if (IsCode(i)) {
            addrInfo.Address = (int32_t)i;
            i += dis->BuildCache(addrInfo, 0, _cpuType) - 1;
        }
    }
}
