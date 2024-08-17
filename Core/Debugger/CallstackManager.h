#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

class Debugger;
class Profiler;
class IConsole;

class CallstackManager {
private:
    Debugger* _debugger;
    std::deque<StackFrameInfo> _callstack;
    std::unique_ptr<Profiler> _profiler;

public:
    CallstackManager(Debugger* debugger, IConsole* console);
    ~CallstackManager();

    void Push(AddressInfo& src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t returnAddress, StackFrameFlags flags);
    void Pop(AddressInfo& dest, uint32_t destAddr);

    [[nodiscard]] bool IsReturnAddrMatch(uint32_t destAddr) const noexcept {
        for (auto it = _callstack.rbegin(); it != _callstack.rend(); ++it) {
            if (it->Return == destAddr) {
                return true;
            }
        }
        return false;
    }

    void GetCallstack(StackFrameInfo* callstackArray, uint32_t& callstackSize);
    int32_t GetReturnAddress();
    Profiler* GetProfiler();

    void Clear();
};
