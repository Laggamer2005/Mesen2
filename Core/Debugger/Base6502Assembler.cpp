#include "pch.h"
#include <regex>
#include <unordered_map>
#include "Utilities/HexUtilities.h"
#include "Utilities/StringUtilities.h"
#include "Debugger/Base6502Assembler.h"
#include "Debugger/DisassemblyInfo.h"
#include "Debugger/LabelManager.h"

// Static regexes can be declared as static members of the class
class Base6502Assembler {
public:
    static const std::regex labelRegex;
    static const std::regex byteRegex;
    // ...

    template<class T>
    void ProcessLine(string code, uint32_t& instructionAddress, vector<int16_t>& output, unordered_map<string, uint32_t>& labels, bool firstPass, unordered_map<string, uint32_t>& currentPassLabels) {
        // Remove comments
        size_t commentOffset = code.find_first_of(';');
        if (commentOffset != string::npos) {
            code = code.substr(0, commentOffset);
        }

        code = StringUtilities::Trim(code);

        std::smatch match;
        if (std::regex_search(code, match, byteRegex)) {
            // Parse .db statements
            vector<string> bytes = StringUtilities::Split(match.str(1) + match.str(3), ' ');
            for (string& byte : bytes) {
                output.push_back((uint8_t)(HexUtilities::FromHex(byte.substr(1))));
                instructionAddress++;
            }
            output.push_back(AssemblerSpecialCodes::EndOfLine);
            return;
        } else if (std::regex_search(code, match, labelRegex)) {
            // Parse label definitions
            string label = match.str(1);
            string afterLabel = match.str(2);
            if (currentPassLabels.find(match.str(1)) != currentPassLabels.end()) {
                output.push_back(AssemblerSpecialCodes::LabelRedefinition);
            } else {
                labels[match.str(1)] = instructionAddress;
                currentPassLabels[match.str(1)] = instructionAddress;
                ProcessLine(afterLabel, instructionAddress, output, labels, firstPass, currentPassLabels);
            }
            return;
        }

        code = StringUtilities::Trim(code);
        if (code.empty()) {
            output.push_back(AssemblerSpecialCodes::EndOfLine);
            return;
        }

        AssemblerLineData op = {};

        size_t opnameOffset = code.find_first_of(' ', 0);
        if (opnameOffset != string::npos) {
            op.OpCode = StringUtilities::ToUpper(code.substr(0, opnameOffset));
            code = StringUtilities::Trim(code.substr(opnameOffset));
            if (code.size() > 0) {
                vector<string> operands = StringUtilities::Split(code, ',');
                for (string& operand : operands) {
                    if (op.OperandCount >= 3) {
                        output.push_back(AssemblerSpecialCodes::InvalidOperands);
                        return;
                    }

                    AssemblerSpecialCodes result = ParseOperand(op, StringUtilities::Trim(operand), firstPass, labels);
                    if (result != AssemblerSpecialCodes::OK) {
                        output.push_back(result);
                        return;
                    }
                    op.OperandCount++;
                }
            }
        } else {
            // no operands could be found
            op.OpCode = StringUtilities::ToUpper(code);
        }

        AssemblerSpecialCodes result = ResolveOpMode(op, instructionAddress, firstPass);
        if (result != AssemblerSpecialCodes::OK) {
            output.push_back(result);
            return;
        }

        AssembleInstruction(op, instructionAddress, output, firstPass);
    }

    template<class T>
    AssemblerSpecialCodes ParseOperand(AssemblerLineData& lineData, string operandStr, bool firstPass, unordered_map<string, uint32_t>& labels) {
        // ...
    }

    template<class T>
    bool IsOpModeAvailable(string& opCode, T addrMode) {
        // ...
    }

    template<class T>
    int16_t GetOpByteCode(string& opCode, T addrMode) {
        // ...
    }

    template<class T>
    void AssembleInstruction(AssemblerLineData& op, uint32_t& instructionAddress, vector<int16_t>& output, bool firstPass) {
        // ...
    }
};

const std::regex Base6502Assembler::labelRegex = std::regex("^\\s*([@_a-zA-Z][@_a-zA-Z0-9+]*):(.*)", std::regex_constants::icase);
const std::regex Base6502Assembler::byteRegex = std::regex("^\\s*[.]db\\s+((\\$[a-fA-F0-9]{1,2}[_])*)(\\$[a-fA-F0-9]{1,2})+\\s*(;*)(.*)$", std::regex_constants::icase);
