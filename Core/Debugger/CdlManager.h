#pragma once
#include "pch.h"
#include "Shared/MemoryType.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

class CodeDataLogger;
class Debugger;
class Disassembler;

class CdlManager {
private:
    std::array<CodeDataLogger*, DebugUtilities::GetMemoryTypeCount()> _codeDataLoggers{};
    Debugger* _debugger = nullptr;
    Disassembler* _disassembler = nullptr;

public:
    CdlManager(Debugger* debugger, Disassembler* disassembler)
        : _debugger(debugger), _disassembler(disassembler) {}

    void GetCdlData(uint32_t offset, uint32_t length, MemoryType memoryType, uint8_t* cdlData);
    int16_t GetCdlFlags(MemoryType memType, uint32_t addr);
    void SetCdlData(MemoryType memType, uint8_t* cdlData, uint32_t length);
    void MarkBytesAs(MemoryType memType, uint32_t start, uint32_t end, uint8_t flags);
    CdlStatistics GetCdlStatistics(MemoryType memType);
    uint32_t GetCdlFunctions(MemoryType memType, uint32_t functions[], uint32_t maxSize);
    void ResetCdl(MemoryType memType);
    void LoadCdlFile(MemoryType memType, const char* cdlFile);
    void SaveCdlFile(MemoryType memType, const char* cdlFile);
    void RegisterCdl(MemoryType memType, CodeDataLogger* cdl);

    void RefreshCodeCache();

    CodeDataLogger* GetCodeDataLogger(MemoryType memType);
};
