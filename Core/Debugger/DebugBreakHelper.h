#pragma once
#include "pch.h"
#include "Debugger/Debugger.h"
#include "Shared/Emulator.h"

class DebugBreakHelper {
private:
    Debugger* _debugger;
    bool _needBreak = false;

public:
    DebugBreakHelper(Debugger* debugger, bool breakBetweenInstructions = false)
        : _debugger(debugger), _needBreak(!debugger->GetEmulator()->IsEmulationThread()) {
        if (_needBreak) {
            breakRequestLoop(breakBetweenInstructions);
        }
    }

    ~DebugBreakHelper() {
        if (_needBreak) {
            _debugger->BreakRequest(true);
        }
    }

private:
    void breakRequestLoop(bool breakBetweenInstructions) {
        while (true) {
            _debugger->BreakRequest(false);
            while (!_debugger->IsExecutionStopped()) {}

            if (breakBetweenInstructions) {
                if (_debugger->GetDebuggerFeatures(_debugger->GetMainCpuType()).ChangeProgramCounter) {
                    break; // Execution stopped in-between 2 main cpu instructions
                } else {
                    // Execution stopped, but in the middle of an instruction, step forward
                    _debugger->Step(_debugger->GetMainCpuType(), 1, StepType::Step, BreakSource::InternalOperation);
                    _debugger->BreakRequest(true);
                    std::this_thread::sleep_for(std::chrono::milliseconds(15));
                }
            } else {
                break; // Execution stopped
            }
        }
    }
};
