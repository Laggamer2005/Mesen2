#include "pch.h"
#include "Debugger/CallstackManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/Profiler.h"

CallstackManager::CallstackManager(Debugger* debugger, IConsole* console)
    : _debugger(debugger), _profiler(std::make_unique<Profiler>(debugger, console)) {}

CallstackManager::~CallstackManager() = default;

void CallstackManager::Push(AddressInfo& src, uint32_t srcAddr, AddressInfo& dest, uint32_t destAddr, AddressInfo& ret, uint32_t returnAddress, StackFrameFlags flags) {
    if (_callstack.size() >= 511) {
        _callstack.pop_front(); // Ensure callstack stays below 512 entries
    }

    StackFrameInfo stackFrame = {
        srcAddr,
        src,
        destAddr,
        dest,
        returnAddress,
        ret,
        flags
    };

    _callstack.emplace_back(std::move(stackFrame));
    _profiler->StackFunction(dest, flags);
}

void CallstackManager::Pop(AddressInfo& dest, uint32_t destAddress) {
    if (_callstack.empty()) {
        return;
    }

    auto& prevFrame = _callstack.back();
    _callstack.pop_back();
    _profiler->UnstackFunction();

    uint32_t returnAddr = prevFrame.Return;

    if (!_callstack.empty() && destAddress != returnAddr) {
        auto it = std::find_if(_callstack.rbegin(), _callstack.rend(), [returnAddr](const auto& frame) {
            return frame.Return == returnAddr;
        });

        if (it != _callstack.rend()) {
            // Found a matching stack frame, unstack until that point
            _callstack.erase(it.base(), _callstack.end());
            _profiler->UnstackFunction(static_cast<size_t>(std::distance(it, _callstack.end())));
        } else {
            // Couldn't find a matching frame, replace the current one
            Push(prevFrame.AbsReturn, returnAddr, dest, destAddress, prevFrame.AbsReturn, returnAddr, StackFrameFlags::None);
        }
    }
}

void CallstackManager::GetCallstack(StackFrameInfo* callstackArray, uint32_t& callstackSize) {
    DebugBreakHelper helper(_debugger);
    callstackSize = static_cast<uint32_t>(_callstack.size());
    std::copy(_callstack.begin(), _callstack.end(), callstackArray);
}

int32_t CallstackManager::GetReturnAddress() {
    DebugBreakHelper helper(_debugger);
    return _callstack.empty() ? -1 : _callstack.back().Return;
}

Profiler* CallstackManager::GetProfiler() {
    return _profiler.get();
}

void CallstackManager::Clear() {
    _callstack.clear();
    _profiler->ResetState();
}
