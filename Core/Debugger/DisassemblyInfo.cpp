#include <cstdint>
#include <stdexcept>
#include <memory>

class DisassemblyInfo {
public:
    DisassemblyInfo() : initialized(false) {}

    DisassemblyInfo(uint32_t cpuAddress, uint8_t cpuFlags, CpuType cpuType, MemoryType memType, MemoryDumper* memoryDumper)
        : cpuType(cpuType), flags(cpuFlags) {
        Initialize(cpuAddress, memType, memoryDumper);
    }

    void Initialize(uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper) {
        byteCode[0] = memoryDumper->GetMemoryValue(memType, cpuAddress);
        opSize = GetOpSize(byteCode[0], flags, cpuType, cpuAddress, memType, memoryDumper);
        for (int i = 1; i < opSize; i++) {
            byteCode[i] = memoryDumper->GetMemoryValue(memType, cpuAddress + i);
        }
        initialized = true;
    }

    bool IsInitialized() const {
        return initialized;
    }

    bool IsValid(uint8_t cpuFlags) const {
        return flags == cpuFlags;
    }

    void Reset() {
        initialized = false;
    }

    void GetDisassembly(std::string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings) const {
        switch (cpuType) {
            case CpuType::Snes:
                SnesDisUtils::GetDisassembly(*this, out, memoryAddr, labelManager, settings);
                break;
            // ...
        }
    }

    EffectiveAddressInfo GetEffectiveAddress(Debugger* debugger, void* cpuState, CpuType cpuType) const {
        switch (cpuType) {
            case CpuType::Snes:
                return SnesDisUtils::GetEffectiveAddress(*this, (SnesConsole*)debugger->GetConsole(), *(SnesCpuState*)cpuState, cpuType);
            // ...
        }
    }

    // ...

private:
    bool initialized;
    CpuType cpuType;
    uint8_t flags;
    uint8_t byteCode[8];
    uint8_t opSize;

    uint8_t GetOpSize(uint32_t opCode, uint8_t flags, CpuType type, uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper) const {
        switch (type) {
            case CpuType::Snes:
                return SnesDisUtils::GetOpSize(opCode, flags);
            // ...
        }
    }

    // ...
};
