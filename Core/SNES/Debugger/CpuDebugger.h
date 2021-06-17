#pragma once
#include "stdafx.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/IDebugger.h"
#include "SNES/CpuTypes.h"

class Disassembler;
class Debugger;
class SnesCpuTraceLogger;
class Cpu;
class CallstackManager;
class MemoryAccessCounter;
class MemoryManager;
class CodeDataLogger;
class EmuSettings;
class ScriptManager;
class IEventManager;
class MemoryMappings;
class BreakpointManager;
class Sa1;
class BaseCartridge;
class Spc;
class Ppu;
class SnesAssembler;
enum class MemoryOperationType;

class CpuDebugger final : public IDebugger
{
	Debugger* _debugger;
	Disassembler* _disassembler;
	MemoryAccessCounter* _memoryAccessCounter;
	MemoryManager* _memoryManager;
	EmuSettings* _settings;
	Cpu* _cpu;
	Sa1* _sa1;
	BaseCartridge* _cart;
	Spc* _spc;
	Ppu* _ppu;
	MemoryMappings* _memoryMappings;

	shared_ptr<CodeDataLogger> _codeDataLogger;
	shared_ptr<IEventManager> _eventManager;
	shared_ptr<SnesAssembler> _assembler;
	shared_ptr<CallstackManager> _callstackManager;
	unique_ptr<BreakpointManager> _breakpointManager;
	unique_ptr<StepRequest> _step;
	unique_ptr<SnesCpuTraceLogger> _traceLogger;

	ITraceLogger* _spcTraceLogger = nullptr;
	ITraceLogger* _dspTraceLogger = nullptr;
	DebuggerFlags _debuggerEnabledFlag = DebuggerFlags::CpuDebuggerEnabled;

	CpuType _cpuType;
	bool _enableBreakOnUninitRead = false;
	uint8_t _prevOpCode = 0xFF;
	uint32_t _prevProgramCounter = 0;

	CpuState& GetCpuState();
	bool IsRegister(uint32_t addr);

public:
	CpuDebugger(Debugger* debugger, CpuType cpuType);

	void Init() override;
	void Reset() override;

	void ProcessRead(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessWrite(uint32_t addr, uint8_t value, MemoryOperationType type);
	void ProcessInterrupt(uint32_t originalPc, uint32_t currentPc, bool forNmi) override;
	void ProcessPpuCycle(int16_t &scanline, uint16_t &cycle);

	void Run() override;
	void Step(int32_t stepCount, StepType type) override;

	ITraceLogger* GetTraceLogger() override;
	BreakpointManager* GetBreakpointManager() override;
	shared_ptr<CallstackManager> GetCallstackManager() override;
	shared_ptr<IAssembler> GetAssembler() override;
	shared_ptr<IEventManager> GetEventManager() override;
	shared_ptr<CodeDataLogger> GetCodeDataLogger() override;

	BaseState& GetState() override;
};