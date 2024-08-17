#pragma once
#include "pch.h"
#include <regex>
#include "Utilities/SimpleLock.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/Debugger.h"
#include "Debugger/IDebugger.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/LabelManager.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/DebugBreakHelper.h"
#include "Debugger/ITraceLogger.h"
#include "Debugger/ExpressionEvaluator.h"
#include "Debugger/TraceLogFileSaver.h"
#include "Utilities/HexUtilities.h"
#include "Shared/Emulator.h"
#include "Shared/EmuSettings.h"

class IConsole;
class Debugger;
class LabelManager;
class MemoryDumper;
class EmuSettings;

enum class RowDataType
{
    Text = 0,
    ByteCode,
    Disassembly,
    EffectiveAddress,
    MemoryValue,
    Align,
    PC,
    A,
    B,
    C,
    D,
    E,
    F,
    H,
    I,
    K,
    L,
    M,
    N,
    R,
    X,
    Y,
    AltA,
    AltB,
    AltC,
    AltD,
    AltE,
    AltF,
    AltH,
    AltL,
    IX,
    IY,
    DB,
    SP,
    PS,
    Cycle,
    Scanline,
    HClock,
    FrameCount,
    CycleCount,

    R0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    Src,
    Dst,

    SFR,

    MAR,
    MDR,
    DPR,
    ML,
    MH,
    PB,
    P,

    RP,
    DP,
    DR,
    SR,
    TR,
    TRB,
};

struct TraceLogPpuState
{
    uint32_t Cycle;
    uint32_t HClock;
    int32_t Scanline;
    uint32_t FrameCount;
};

struct RowPart
{
    RowDataType DataType;
    string Text;
    bool DisplayInHex;
    int MinWidth;
};

template<typename TraceLoggerType, typename CpuStateType>
class BaseTraceLogger : public ITraceLogger
{
protected:
    static constexpr int ExecutionLogSize = 30000;

    TraceLoggerOptions _options;
    IConsole* _console;
    EmuSettings* _settings;
    LabelManager* _labelManager;
    MemoryDumper* _memoryDumper;
    Debugger* _debugger;

    CpuType _cpuType = CpuType::Snes;
    MemoryType _cpuMemoryType = MemoryType::SnesMemory;

    vector<RowPart> _rowParts;

    uint32_t _currentPos = 0;

    bool _pendingLog = false;
    CpuStateType _lastState = {};
    DisassemblyInfo _lastDisassemblyInfo = {};

    CpuStateType* _cpuState = nullptr;
    DisassemblyInfo *_disassemblyCache = nullptr;
    uint64_t* _rowIds = nullptr;
    TraceLogPpuState* _ppuState = nullptr;

    unique_ptr<ExpressionEvaluator> _expEvaluator;
    ExpressionData _conditionData;

    // Other helper methods are omitted for brevity

public:
    BaseTraceLogger(Debugger* debugger, IDebugger* cpuDebugger, CpuType cpuType)
        : _debugger(debugger), _console(debugger->GetConsole()), _settings(debugger->GetEmulator()->GetSettings()),
          _labelManager(debugger->GetLabelManager()), _memoryDumper(debugger->GetMemoryDumper()), _cpuType(cpuType),
          _cpuMemoryType(DebugUtilities::GetCpuMemoryType(cpuType)), _enabled(false)
    {
        _disassemblyCache = new DisassemblyInfo[ExecutionLogSize];
        _rowIds = new uint64_t[ExecutionLogSize];
        _ppuState = new TraceLogPpuState[ExecutionLogSize];
        _cpuState = new CpuStateType[ExecutionLogSize];
    }

    virtual ~BaseTraceLogger()
    {
        delete[] _disassemblyCache;
        delete[] _rowIds;
        delete[] _ppuState;
        delete[] _cpuState;
    }

    void Clear() override
    {
        _currentPos = 0;
        memset(_rowIds, 0, sizeof(uint64_t) * ExecutionLogSize);
    }

    void LogNonExec(MemoryOperationInfo& operation, AddressInfo& addressInfo)
    {
        if (_pendingLog)
        {
            int pos = _currentPos - 1;
            if (pos < 0)
                pos = ExecutionLogSize - 1;

            if (ConditionMatches(_lastDisassemblyInfo, operation, addressInfo))
            {
                AddRow(_lastState, _lastDisassemblyInfo);
                _pendingLog = false;
            }
        }
    }

    void Log(CpuStateType& cpuState, DisassemblyInfo& disassemblyInfo, MemoryOperationInfo& operation, AddressInfo& addressInfo)
    {
        if (_enabled)
        {
            if (ConditionMatches(disassemblyInfo, operation, addressInfo))
            {
                AddRow(cpuState, disassemblyInfo);
            }
            else
            {
                _pendingLog = true;
                _lastState = cpuState;
                _lastDisassemblyInfo = disassemblyInfo;
            }
        }
    }

    void SetOptions(TraceLoggerOptions options) override
    {
        DebugBreakHelper helper(_debugger);
        _options = options;

        _enabled = options.Enabled;

        string condition = _options.Condition;
        string format = _options.Format;

        _conditionData = ExpressionData();
        if (!condition.empty())
        {
            bool success = false;
            ExpressionData rpnList = _expEvaluator->GetRpnList(condition, success);
            if (success)
            {
                _conditionData = rpnList;
            }
        }

        ParseFormatString(format);

        _debugger->ProcessConfigChange();
    }

    int64_t GetRowId(uint32_t offset) override
    {
        int32_t pos = ((int32_t)_currentPos - (int32_t)offset);
        int32_t i = (pos > 0 ? pos : ExecutionLogSize + pos) - 1;
        if (!_disassemblyCache[i].IsInitialized())
        {
            return -1;
        }
        return _rowIds[i];
    }

    bool ConditionMatches(DisassemblyInfo &disassemblyInfo, MemoryOperationInfo &operationInfo, AddressInfo& addressInfo)
    {
        if (!_conditionData.RpnQueue.empty())
        {
            EvalResultType type;
            if (!_expEvaluator->Evaluate(_conditionData, type, operationInfo, addressInfo))
            {
                return false;
            }
        }
        return true;
    }

    void GetExecutionTrace(TraceRow& row, uint32_t offset) override
    {
        int pos = ((int)_currentPos - offset);
        int index = (pos > 0 ? pos : ExecutionLogSize + pos) - 1;

        CpuStateType& state = _cpuState[index];
        string logOutput;
        logOutput.reserve(300);
        ((TraceLoggerType*)this)->GetTraceRow(logOutput, state, _ppuState[index], _disassemblyCache[index]);

        row.Type = _cpuType;
        _disassemblyCache[index].GetByteCode(row.ByteCode);
        row.ByteCodeSize = _disassemblyCache[index].GetOpSize();
        row.ProgramCounter = ((TraceLoggerType*)this)->GetProgramCounter(state);
        row.LogSize = std::min<uint32_t>(499, (uint32_t)logOutput.size());
        memcpy(row.LogOutput, logOutput.c_str(), row.LogSize);
        row.LogOutput[row.LogSize] = 0;
    }
};
