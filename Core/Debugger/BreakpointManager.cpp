// BreakpointManager.h
#pragma once
#include "Debugger/DebugTypes.h"
#include "Debugger/Debugger.h"
#include "Shared/MemoryOperationType.h"

class BreakpointManager {
public:
    BreakpointManager(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType, BaseEventManager* eventManager);
    void SetBreakpoints(Breakpoint breakpoints[], uint32_t count);
    int InternalCheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo& address, bool processMarkedBreakpoints);

private:
    Debugger* _debugger;
    IDebugger* _cpuDebugger;
    CpuType _cpuType;
    bool _hasBreakpoint;
    std::unique_ptr<ExpressionEvaluator> _bpExpEval;
    std::vector<std::vector<Breakpoint>> _breakpoints;
    std::vector<std::vector<ExpressionData>> _rpnList;
    bool _hasBreakpointType[BreakpointTypeCount];
    BaseEventManager* _eventManager;
};

// BreakpointManager.cpp
#include "BreakpointManager.h"
#include "Debugger/Breakpoint.h"
#include "Debugger/DebugUtilities.h"
#include "ExpressionEvaluator.h"

BreakpointManager::BreakpointManager(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType, BaseEventManager* eventManager)
    : _debugger(debugger), _cpuDebugger(cpuDebugger), _cpuType(cpuType), _hasBreakpoint(false), _eventManager(eventManager) {
    _bpExpEval.reset(new ExpressionEvaluator(_debugger, _cpuDebugger, _cpuType));
}

void BreakpointManager::SetBreakpoints(Breakpoint breakpoints[], uint32_t count) {
    _hasBreakpoint = false;
    for (int i = 0; i < BreakpointTypeCount; i++) {
        _breakpoints[i].clear();
        _rpnList[i].clear();
        _hasBreakpointType[i] = false;
    }

    for (uint32_t j = 0; j < count; j++) {
        Breakpoint& bp = breakpoints[j];
        for (int i = 0; i < BreakpointTypeCount; i++) {
            MemoryOperationType opType = (MemoryOperationType)i;
            if ((bp.IsMarked() || bp.IsEnabled()) && bp.HasBreakpointType(GetBreakpointType(opType))) {
                // ...
            }
        }
    }
}

BreakpointType BreakpointManager::GetBreakpointType(MemoryOperationType type) {
    static const std::map<MemoryOperationType, BreakpointType> typeMapping = {
        {MemoryOperationType::ExecOperand, BreakpointType::Execute},
        {MemoryOperationType::ExecOpCode, BreakpointType::Execute},
        // ...
    };
    return typeMapping.at(type);
}

template <uint8_t accessWidth>
int BreakpointManager::InternalCheckBreakpoint(MemoryOperationInfo operationInfo, AddressInfo& address, bool processMarkedBreakpoints) {
    EvalResultType resultType;
    const auto& breakpoints = _breakpoints[(int)operationInfo.Type];
    for (size_t i = 0, len = breakpoints.size(); i < len; i++) {
        if (breakpoints[i].Matches<accessWidth>(operationInfo, address)) {
            if (breakpoints[i].HasCondition() && !_bpExpEval->Evaluate(_rpnList[(int)operationInfo.Type][i], resultType, operationInfo, address)) {
                continue;
            }

            if (breakpoints[i].IsMarked() && processMarkedBreakpoints) {
                _eventManager->AddEvent(DebugEventType::Breakpoint, operationInfo, breakpoints[i].GetId());
            }
            if (breakpoints[i].IsEnabled()) {
                return breakpoints[i].GetId();
            }
        }
    }
    return -1;
}
