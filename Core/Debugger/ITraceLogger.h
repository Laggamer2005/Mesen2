#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"

struct TraceRow {
    uint32_t programCounter; // snake_case for better readability
    CpuType type;
    std::array<uint8_t, 8> byteCode; // use std::array for bounded arrays
    uint8_t byteCodeSize;
    uint32_t logSize;
    char logOutput[500]; // consider using std::string for dynamic strings
};

struct TraceLoggerOptions {
    bool enabled;
    bool indentCode;
    bool useLabels;
    std::string condition; // use std::string for dynamic strings
    std::string format;
};

class ITraceLogger {
protected:
    bool enabled_ = false; // trailing underscore for member variables

public:
    static uint64_t nextRowId;

    virtual int64_t getRowId(uint32_t offset) = 0;
    virtual void getExecutionTrace(TraceRow& row, uint32_t offset) = 0;
    virtual void clear() = 0;
    virtual void setOptions(TraceLoggerOptions options) = 0;

    [[nodiscard]] bool isEnabled() const { return enabled_; } // const correctness
};
