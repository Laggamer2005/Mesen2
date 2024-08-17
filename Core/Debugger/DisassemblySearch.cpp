#include "pch.h"
#include "Debugger/Disassembler.h"
#include "Debugger/DisassemblySearch.h"
#include "Debugger/LabelManager.h"

DisassemblySearch::DisassemblySearch(Disassembler* disassembler, LabelManager* labelManager)
    : _disassembler(disassembler), _labelManager(labelManager) {}

int32_t DisassemblySearch::SearchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options) {
    CodeLineData results[1] = {};
    uint32_t resultCount = SearchDisassembly(cpuType, searchString, startAddress, options, results, 1);
    return resultCount > 0 ? results[0].Address : -1;
}

uint32_t DisassemblySearch::FindOccurrences(CpuType cpuType, const char* searchString, DisassemblySearchOptions options, CodeLineData output[], uint32_t maxResultCount) {
    return SearchDisassembly(cpuType, searchString, 0, options, output, maxResultCount);
}

uint32_t DisassemblySearch::SearchDisassembly(CpuType cpuType, const char* searchString, int32_t startAddress, DisassemblySearchOptions options, CodeLineData searchResults[], uint32_t maxResultCount) {
    MemoryType memType = DebugUtilities::GetCpuMemoryType(cpuType);
    uint16_t bank = startAddress >> 16;
    uint16_t maxBank = _disassembler->GetMaxBank(cpuType);

    vector<DisassemblyResult> rows;
    int32_t startRow = 0;

    do {
        rows = _disassembler->Disassemble(cpuType, bank);
        if (rows.empty()) {
            return 0;
        }

        if (options.SearchBackwards) {
            startRow = rows.size() - 1;
        } else {
            startRow = 0;
        }

        int32_t prevAddress = -1;
        uint32_t resultCount = 0;

        for (int i = startRow; i >= 0 && i < rows.size(); i += options.SearchBackwards ? -1 : 1) {
            if (rows[i].CpuAddress < 0) {
                continue;
            }

            if ((options.SearchBackwards && prevAddress > startAddress && rows[i].CpuAddress <= startAddress) ||
                (!options.SearchBackwards && prevAddress < startAddress && rows[i].CpuAddress >= startAddress) ||
                i > 500000) {
                if (i > 0) {
                    // Checked entire memory space without finding a match (or checked over 500k rows), give up
                    return resultCount;
                }
            }

            prevAddress = rows[i].CpuAddress;

            CodeLineData lineData;
            _disassembler->GetLineData(rows[i], cpuType, memType, lineData);

            if (TextContains(searchString, lineData.Text, 1000, options) ||
                TextContains(searchString, lineData.Comment, 1000, options) ||
                lineData.EffectiveAddress.ShowAddress && lineData.EffectiveAddress.Address >= 0 &&
                TextContains(searchString, _labelManager->GetLabel({(int32_t)lineData.EffectiveAddress.Address, lineData.EffectiveAddress.Type}).c_str(), 1000, options) ||
                maxResultCount == 1 && lineData.EffectiveAddress.ValueSize > 0 &&
                TextContains(searchString, "$" + HexUtilities::ToHex(lineData.Value).c_str(), 1000, options)) {
                searchResults[resultCount] = lineData;
                if (++resultCount == maxResultCount) {
                    return resultCount;
                }
            }
        }

        // No match found, go to next/previous bank
        bank += options.SearchBackwards ? -1 : 1;
        if (bank < 0) {
            bank = maxBank;
        } else if (bank > maxBank) {
            if (startAddress == 0) {
                return resultCount;
            }
            bank = 0;
        }
    } while (true);

    return resultCount;
}

bool DisassemblySearch::TextContains(string& needle, const char* hay, int size, DisassemblySearchOptions& options) {
    return TextContainsImpl(needle, hay, size, options, options.MatchCase);
}

template<bool matchCase>
bool DisassemblySearch::TextContainsImpl(string& needle, const char* hay, int size, DisassemblySearchOptions& options) {
    int pos = 0;
    for (int j = 0; j < size; j++) {
        char c = hay[j];
        if (c <= 0) {
            break;
        }

        if (needle[pos] == (matchCase ? c : tolower(c))) {
            if (options.MatchWholeWord && pos == 0 && j > 0 && !IsWordSeparator(hay[j_-_1])) {
                continue;
            }

            pos++;
            if (pos == needle.size()) {
                if (options.MatchWholeWord && j < size - 1 && !IsWordSeparator(hay[j_+_1])) {
                    j -= pos - 1;
                    pos = 0;
                    continue;
                }
                return true;
            }
        } else {
            j -= pos;
            pos = 0;
        }
    }
    return false;
}

bool DisassemblySearch::IsWordSeparator(char c) {
    return !((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_' || c == '@' || c == '$' || c == '#');
}
