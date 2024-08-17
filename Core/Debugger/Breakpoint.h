#pragma once
#include "pch.h"

enum class CpuType : uint8_t;
enum class MemoryType;
struct AddressInfo;
enum class BreakpointType;
enum class BreakpointTypeFlags;
enum class MemoryOperationType;
struct MemoryOperationInfo;

class Breakpoint {
public:
    Breakpoint(uint32_t id, CpuType cpuType, MemoryType memoryType, BreakpointTypeFlags type, int32_t startAddr, int32_t endAddr)
        : _id(id), _cpuType(cpuType), _memoryType(memoryType), _type(type), _startAddr(startAddr), _endAddr(endAddr), _enabled(false), _markEvent(false), _ignoreDummyOperations(false) {}

    template<uint8_t accessWidth = 1> bool Matches(MemoryOperationInfo& opInfo, AddressInfo& info);
    bool HasBreakpointType(BreakpointType type);
    std::string GetCondition();
    bool HasCondition();

    uint32_t GetId() const { return _id; }
    CpuType GetCpuType() const { return _cpuType; }
    bool IsEnabled() const { return _enabled; }
    bool IsMarked() const { return _markEvent; }
    bool IsAllowedForOpType(MemoryOperationType opType);

    void Enable() { _enabled = true; }
    void Disable() { _enabled = false; }
    void MarkEvent() { _markEvent = true; }
    void UnmarkEvent() { _markEvent = false; }

private:
    uint32_t _id;
    CpuType _cpuType;
    MemoryType _memoryType;
    BreakpointTypeFlags _type;
    int32_t _startAddr;
    int32_t _endAddr;
    bool _enabled;
    bool _markEvent;
    bool _ignoreDummyOperations;
    std::string _condition; // Use std::string instead of char array for dynamic memory allocation
};
