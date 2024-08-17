#pragma once
#include "pch.h"
#include "Debugger/AddressInfo.h"
#include "Shared/MemoryType.h"

class IConsole;
class MemoryDumper;
class LabelManager;
class Debugger;
class EmuSettings;

enum class MemoryType;
enum class CpuType : uint8_t;

struct EffectiveAddressInfo {
    int64_t Address = -1;
    MemoryType Type = MemoryType::None;
    uint8_t ValueSize = 0;
    bool ShowAddress = false;

    EffectiveAddressInfo(int64_t address = -1, uint8_t valueSize = 1, bool showAddress = true, MemoryType memType = MemoryType::None)
        : Address(address), Type(memType), ValueSize(valueSize), ShowAddress(showAddress) {}
};

class DisassemblyInfo {
private:
    uint8_t _byteCode[8]{};
    uint8_t _opSize{};
    uint8_t _flags{};
    CpuType _cpuType{};
    bool _initialized = false;

    template <CpuType type> uint32_t GetFullOpCode();

public:
    DisassemblyInfo() = default;
    DisassemblyInfo(uint32_t cpuAddress, uint8_t cpuFlags, CpuType cpuType, MemoryType memType, MemoryDumper* memoryDumper)
        : _cpuType(cpuType) {
        Initialize(cpuAddress, cpuFlags, cpuType, memType, memoryDumper);
    }

    void Initialize(uint32_t cpuAddress, uint8_t cpuFlags, CpuType cpuType, MemoryType memType, MemoryDumper* memoryDumper);
    bool IsInitialized() const { return _initialized; }
    bool IsValid(uint8_t cpuFlags);
    void Reset() { _initialized = false; }

    void GetDisassembly(std::string& out, uint32_t memoryAddr, LabelManager* labelManager, EmuSettings* settings);

    CpuType GetCpuType() const { return _cpuType; }
    uint8_t GetOpCode() const { return _byteCode[0]; }
    uint8_t GetOpSize() const { return _opSize; }
    uint8_t GetFlags() const { return _flags; }
    uint8_t* GetByteCode() { return _byteCode; }

    void GetByteCode(uint8_t copyBuffer[8]) {
        std::copy(std::begin(_byteCode), std::end(_byteCode), copyBuffer);
    }

    void GetByteCode(std::string& out) {
        out.assign(reinterpret_cast<char*>(_byteCode), 8);
    }

    static uint8_t GetOpSize(uint32_t opCode, uint8_t flags, CpuType type, uint32_t cpuAddress, MemoryType memType, MemoryDumper* memoryDumper);

    bool IsJumpToSub() const;
    bool IsReturnInstruction() const;

    bool CanDisassembleNextOp() const;

    bool IsUnconditionalJump() const;
    bool IsJump() const;
    void UpdateCpuFlags(uint8_t& cpuFlags);

    EffectiveAddressInfo GetEffectiveAddress(Debugger* debugger, void* cpuState, CpuType type);

    uint32_t GetMemoryValue(EffectiveAddressInfo effectiveAddress, MemoryDumper* memoryDumper, MemoryType memType);

    // Consider using constexpr for GetFullOpCode() and making it a static function
    // if it doesn't depend on instance variables.
};
