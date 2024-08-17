#pragma once
#include "pch.h"

enum class VectorType : uint8_t { // Use uint8_t as the underlying type to save space
    Indirect,
    Direct,
};

struct CpuVectorDefinition {
    char Name[15] = {}; // Consider using std::string or const char* instead of a fixed-size array
    uint32_t Address = 0;
    VectorType Type = VectorType::Indirect;
};

struct DebuggerFeatures {
    bool RunToIrq : 1; // Use bit fields to save space
    bool RunToNmi : 1;
    bool StepOver : 1;
    bool StepOut : 1;
    bool StepBack : 1;
    bool ChangeProgramCounter : 1;
    bool CallStack : 1;
    bool CpuCycleStep : 1;

    uint8_t CpuVectorCount;
    CpuVectorDefinition CpuVectors[10]; // Consider using a std::vector instead of a fixed-size array
};
