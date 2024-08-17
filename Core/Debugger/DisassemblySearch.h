#pragma once
#include "pch.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugUtilities.h"

class Disassembler;
class LabelManager;
enum class CpuType : uint8_t;

struct DisassemblySearchOptions {
    bool matchCase;
    bool matchWholeWord;
    bool searchBackwards;
    bool skipFirstLine;
};

class DisassemblySearch {
private:
    Disassembler* disassembler;
    LabelManager* labelManager;

    uint32_t searchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options, CodeLineData searchResults[], uint32_t maxResultCount);

    bool textContains(const std::string& needle, const char* hay, int size, DisassemblySearchOptions& options) const;
    bool isWordSeparator(char c) const;

public:
    DisassemblySearch(Disassembler* disassembler, LabelManager* labelManager);

    int32_t searchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options);
    uint32_t findOccurrences(CpuType cpuType, const char* searchString, DisassemblySearchOptions options, CodeLineData output[], uint32_t maxResultCount);
};
