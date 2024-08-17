#include "pch.h"
#include "Debugger/Breakpoint.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

template<uint8_t accessWidth>
bool Breakpoint::Matches(MemoryOperationInfo& operation, AddressInfo& info) {
    if (operation.MemType == _memoryType && DebugUtilities::IsRelativeMemory(_memoryType)) {
        for (int i = 0; i < accessWidth; i++) {
            if ((_startAddr <= (int32_t)operation.Address + i) && ((int32_t)operation.Address + i <= _endAddr)) {
                return true;
            }
        }
    } else if (_memoryType == info.Type) {
        for (int i = 0; i < accessWidth; i++) {
            if ((_startAddr <= info.Address + i) && (info.Address + i <= _endAddr)) {
                return true;
            }
        }
    }
    return false;
}

bool Breakpoint::HasBreakpointType(BreakpointType type) {
    return (_type & (uint8_t)type) != 0;
}

string Breakpoint::GetCondition() {
    return _condition;
}

bool Breakpoint::HasCondition() {
    return !_condition.empty();
}

uint32_t Breakpoint::GetId() {
    return _id;
}

CpuType Breakpoint::GetCpuType() {
    return _cpuType;
}

bool Breakpoint::IsEnabled() {
    return _enabled;
}

bool Breakpoint::IsMarked() {
    return _markEvent;
}

bool Breakpoint::IsAllowedForOpType(MemoryOperationType opType) {
    return !_ignoreDummyOperations || (opType != MemoryOperationType::DummyRead && opType != MemoryOperationType::DummyWrite);
}

template bool Breakpoint::Matches<1>(MemoryOperationInfo& operation, AddressInfo& info);
template bool Breakpoint::Matches<2>(MemoryOperationInfo& operation, AddressInfo& info);
template bool Breakpoint::Matches<4>(MemoryOperationInfo& operation, AddressInfo& info);
