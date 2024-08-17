#include "pch.h"
#include <limits>
#include "Debugger/Profiler.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/Debugger.h"
#include "Debugger/MemoryDumper.h"
#include "Debugger/DebugTypes.h"
#include "Shared/Interfaces/IConsole.h"

static constexpr int32_t ResetFunctionIndex = -1;

Profiler::Profiler(Debugger* debugger, IConsole* console)
    : _debugger(debugger), _console(console) {
    InternalReset();
}

Profiler::~Profiler() {}

void Profiler::StackFunction(AddressInfo& addr, StackFrameFlags stackFlag) {
    if (addr.Address >= 0) {
        uint32_t key = addr.Address | ((uint8_t)addr.Type << 24);
        if (_functions.find(key) == _functions.end()) {
            _functions[key] = ProfiledFunction();
            _functions[key].Address = addr;
        }

        UpdateCycles();

        _stackFlags.emplace_back(stackFlag);
        _cycleCountStack.emplace_back(_currentCycleCount);
        _functionStack.emplace_back(_currentFunction);

        if (_functionStack.size() > 100) {
            _functionStack.pop_front();
            _cycleCountStack.pop_front();
            _stackFlags.pop_front();
        }

        _functions[key].CallCount++;

        _currentFunction = key;
        _currentCycleCount = 0;
    }
}

void Profiler::UpdateCycles() {
    uint64_t masterClock = _console->GetMasterClock();
    uint64_t clockGap = masterClock - _prevMasterClock;

    ProfiledFunction& func = _functions[_currentFunction];
    func.ExclusiveCycles += clockGap;
    func.InclusiveCycles += clockGap;

    for (auto it = _functionStack.rbegin(); it != _functionStack.rend(); ++it) {
        _functions[*it].InclusiveCycles += clockGap;
        if (_stackFlags[_functionStack.size()_-_1_-_std::distance(_functionStack.begin(),_it)] != StackFrameFlags::None) {
            break;
        }
    }

    _currentCycleCount += clockGap;
    _prevMasterClock = masterClock;
}

void Profiler::UnstackFunction() {
    if (!_functionStack.empty()) {
        UpdateCycles();

        ProfiledFunction& func = _functions[_currentFunction];
        func.MinCycles = std::min(func.MinCycles, _currentCycleCount);
        func.MaxCycles = std::max(func.MaxCycles, _currentCycleCount);

        _currentFunction = _functionStack.back();
        _functionStack.pop_back();
        _stackFlags.pop_back();
        _currentCycleCount = _cycleCountStack.back() + _currentCycleCount;
        _cycleCountStack.pop_back();
    }
}

void Profiler::Reset() {
    DebugBreakHelper helper(_debugger);
    InternalReset();
}

void Profiler::ResetState() {
    _prevMasterClock = _console->GetMasterClock();
    _currentCycleCount = 0;
    _functionStack.clear();
    _stackFlags.clear();
    _cycleCountStack.clear();
    _currentFunction = ResetFunctionIndex;
}

void Profiler::InternalReset() {
    ResetState();
    _functions.clear();
    _functions[ResetFunctionIndex] = ProfiledFunction();
    _functions[ResetFunctionIndex].Address = { ResetFunctionIndex, MemoryType::None };
}

void Profiler::GetProfilerData(ProfiledFunction* profilerData, uint32_t& functionCount) {
    DebugBreakHelper helper(_debugger);

    UpdateCycles();

    functionCount = 0;
    for (auto& func : _functions) {
        profilerData[functionCount] = func.second;
        functionCount++;

        if (functionCount >= 100000) {
            break;
