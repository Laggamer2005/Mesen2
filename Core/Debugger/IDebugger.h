#pragma once
#include "pch.h"
#include "Debugger/DebuggerFeatures.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/StepBackManager.h"
#include "Debugger/FrozenAddressManager.h"

class IDebugger {
protected:
    std::unique_ptr<StepRequest> step_;
    std::unique_ptr<StepBackManager> stepBackManager_;
    FrozenAddressManager frozenAddressManager_;

public:
    bool ignoreBreakpoints = false;
    bool allowChangeProgramCounter = false;
    CpuInstructionProgress instructionProgress = {};

    IDebugger(Emulator* emulator) 
        : stepBackManager_(std::make_unique<StepBackManager>(emulator, this)) {}

    virtual ~IDebugger() = default;

    StepRequest* getStepRequest() { return step_.get(); }
    bool checkStepBack() { return stepBackManager_->checkStepBack(); }
    bool isStepBack() { return stepBackManager_->isRewinding(); }
    void resetStepBackCache() { stepBackManager_->resetCache(); }
    void stepBack(int32_t stepCount) { stepBackManager_->stepBack(static_cast<StepBackType>(stepCount)); }
    virtual StepBackConfig getStepBackConfig() { return { getCpuCycleCount(), 0, 0 }; }

    FrozenAddressManager& getFrozenAddressManager() { return frozenAddressManager_; }

    virtual void resetPrevOpcode() {}

    virtual void step(int32_t stepCount, StepType type) = 0;
    virtual void reset() = 0;
    virtual void run() = 0;

    virtual void init() {}
    virtual void processConfigChange() {}

    virtual void processInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) {}
    virtual void processInputOverrides(DebugControllerState inputOverrides[8]) {}

    virtual void drawPartialFrame() {}

    virtual DebuggerFeatures getSupportedFeatures() { return {}; }
    virtual uint64_t getCpuCycleCount() { return 0; }
    virtual uint32_t getProgramCounter(bool getInstPc) = 0;
    virtual void setProgramCounter(uint32_t addr, bool updateDebuggerOnly = false) = 0;

    virtual uint8_t getCpuFlags() { return 0; }

    virtual BreakpointManager* getBreakpointManager() = 0;
    virtual CallstackManager* getCallstackManager() = 0;
    virtual IAssembler* getAssembler() = 0;
    virtual BaseEventManager* getEventManager() = 0;
    virtual ITraceLogger* getTraceLogger() = 0;
    virtual PpuTools* getPpuTools() { return nullptr; }

    virtual BaseState& getState() = 0;
    virtual void getPpuState(BaseState& state) {}
    virtual void setPpuState(BaseState& state) {}
};
