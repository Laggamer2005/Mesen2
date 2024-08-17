#pragma once
#include "pch.h"
#include <unordered_map>
#include <functional>
#include "Debugger/DebugTypes.h"

class Debugger;

class AddressHasher {
public:
    size_t operator()(uint64_t addr) const noexcept { // added noexcept
        return addr; // no need for const reference, as uint64_t is primitive
    }
};

struct LabelInfo {
    std::string Label;
    std::string Comment;
};

class LabelManager {
private:
    std::unordered_map<uint64_t, LabelInfo, AddressHasher> _codeLabels;
    std::unordered_map<std::string, uint64_t> _codeLabelReverseLookup;

    Debugger* _debugger;

    int64_t GetLabelKey(uint32_t absoluteAddr, MemoryType memType);
    MemoryType GetKeyMemoryType(uint64_t key);
    bool InternalGetLabel(AddressInfo address, std::string& label);

public:
    explicit LabelManager(Debugger* debugger) : _debugger{debugger} {} // added explicit and initializer list

    void SetLabel(uint32_t address, MemoryType memType, const std::string& label, const std::string& comment);
    void ClearLabels();

    AddressInfo GetLabelAbsoluteAddress(const std::string& label) const; // added const
    int32_t GetLabelRelativeAddress(const std::string& label, CpuType cpuType);

    std::string GetLabel(AddressInfo address, bool checkRegisterLabels = true) const; // added const
    std::string GetComment(AddressInfo absAddress) const; // added const
    bool GetLabelAndComment(AddressInfo address, LabelInfo& label);

    bool ContainsLabel(const std::string& label) const; // added const
    bool HasLabelOrComment(AddressInfo address) const; // added const
};
