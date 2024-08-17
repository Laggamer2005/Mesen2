#pragma once
#include "pch.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

class IConsole;
class Debugger;
class LabelManager;
class CodeDataLogger;
class MemoryDumper;
class DisassemblySearch;
class EmuSettings;
struct SnesCpuState;
enum class CpuType : uint8_t;

struct DisassemblerSource {
    std::vector<DisassemblyInfo> cache;
    uint32_t size = 0;
};

class Disassembler {
private:
    friend class DisassemblySearch;

    IConsole* console_;
    EmuSettings* settings_;
    Debugger* debugger_;
    LabelManager* labelManager_;
    MemoryDumper* memoryDumper_;

    DisassemblerSource sources_[DebugUtilities::GetMemoryTypeCount()] = {};

    void initSource(MemoryType type);
    DisassemblerSource& getSource(MemoryType type);

    void getLineData(DisassemblyResult& result, CpuType type, MemoryType memType, CodeLineData& data);
    int32_t getMatchingRow(std::vector<DisassemblyResult>& rows, uint32_t address, bool returnFirstRow);
    std::vector<DisassemblyResult> disassemble(CpuType cpuType, uint16_t bank);
    uint16_t getMaxBank(CpuType cpuType);

public:
    Disassembler(IConsole* console, Debugger* debugger);

    uint32_t buildCache(AddressInfo& addrInfo, uint8_t cpuFlags, CpuType type);
    void resetPrgCache();
    void invalidateCache(AddressInfo addrInfo, CpuType type);

    inline DisassemblyInfo getDisassemblyInfo(AddressInfo& info, uint32_t cpuAddress, uint8_t cpuFlags, CpuType type) {
        DisassemblyInfo disassemblyInfo;
        if (info.Address >= 0) {
            disassemblyInfo = getSource(info.Type).cache[info.Address];
        }

        if (!disassemblyInfo.isInitialized()) {
            disassemblyInfo.initialize(cpuAddress, cpuFlags, type, DebugUtilities::GetCpuMemoryType(type), memoryDumper_);
        }
        return disassemblyInfo;
    }

    uint32_t getDisassemblyOutput(CpuType type, uint32_t address, CodeLineData output[], uint32_t rowCount);
    int32_t getDisassemblyRowAddress(CpuType type, uint32_t address, int32_t rowOffset);
};
