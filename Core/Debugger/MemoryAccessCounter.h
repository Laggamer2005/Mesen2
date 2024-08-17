#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"
#include "Shared/MemoryType.h"

class Debugger;
class SnesMemoryManager;
class Spc;
class SnesConsole;
class Sa1;
class Gsu;
class Cx4;
class Gameboy;

struct AddressCounters {
    uint64_t readStamp;
    uint64_t writeStamp;
    uint64_t execStamp;
    uint32_t readCounter;
    uint32_t writeCounter;
    uint32_t execCounter;
};

enum class ReadResult : uint8_t {
    Normal,
    FirstUninitRead,
    UninitRead
};

class MemoryAccessCounter {
private:
    std::vector<AddressCounters> counters_[DebugUtilities::GetMemoryTypeCount()];
    Debugger* debugger_;
    bool enableBreakOnUninitRead_;

public:
    MemoryAccessCounter(Debugger* debugger) : debugger_(debugger), enableBreakOnUninitRead_(false) {}

    template<uint8_t accessWidth = 1>
    ReadResult ProcessMemoryRead(AddressInfo& addressInfo, uint64_t masterClock) {
        // Implementation
    }

    template<uint8_t accessWidth = 1>
    void ProcessMemoryWrite(AddressInfo& addressInfo, uint64_t masterClock) {
        // Implementation
    }

    template<uint8_t accessWidth = 1>
    void ProcessMemoryExec(AddressInfo& addressInfo, uint64_t masterClock) {
        // Implementation
    }

    void ResetCounts() {
        // Implementation
    }

    void GetAccessCounts(uint32_t offset, uint32_t length, MemoryType memoryType, AddressCounters counts[]) {
        // Implementation
    }
};
