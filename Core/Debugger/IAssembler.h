#pragma once
#include "pch.h"

// Use a namespace for your custom enumerations and classes
namespace Assembler {

enum class AssemblerResult : int16_t {
    OK = 0,
    EndOfLine,
    ParsingError,
    OutOfRangeJump,
    LabelRedefinition,
    MissingOperand,
    OperandOutOfRange,
    InvalidHex,
    InvalidSpaces,
    TrailingText,
    UnknownLabel,
    InvalidInstruction,
    InvalidBinaryValue,
    InvalidOperands,
    InvalidLabel
};

class IAssembler {
public:
    virtual ~IAssembler() {}
    virtual int16_t AssembleCode(const std::string& code, uint32_t startAddress, int16_t* assembledCode) = 0;
};

}  // namespace Assembler
