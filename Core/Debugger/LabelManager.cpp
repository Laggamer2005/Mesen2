#include "pch.h"
#include "Debugger/LabelManager.h"
#include "Debugger/Debugger.h"
#include "Debugger/DebugUtilities.h"
#include "Debugger/DebugBreakHelper.h"

LabelManager::LabelManager(Debugger *debugger) : _debugger(debugger) {}

void LabelManager::ClearLabels() {
    DebugBreakHelper helper(_debugger);
    _codeLabels.clear();
    _codeLabelReverseLookup.clear();
}

void LabelManager::SetLabel(uint32_t address, MemoryType memType, std::string label, std::string comment) {
    DebugBreakHelper helper(_debugger);
    uint64_t key = GetLabelKey(address, memType);

    auto it = _codeLabels.find(key);
    if (it != _codeLabels.end()) {
        _codeLabelReverseLookup.erase(it->second.Label);
    }

    if (!label.empty() || !comment.empty()) {
        label = label.substr(0, 400); // Restrict labels to 400 bytes
        LabelInfo labelInfo{ label, comment };
        _codeLabels.emplace(key, labelInfo);
        _codeLabelReverseLookup.emplace(label, key);
    } else {
        _codeLabels.erase(key);
    }
}

int64_t LabelManager::GetLabelKey(uint32_t absoluteAddr, MemoryType memType) {
    return absoluteAddr | ((uint64_t)memType << 32);
}

MemoryType LabelManager::GetKeyMemoryType(uint64_t key) {
    return (MemoryType)(key >> 32);
}

std::string LabelManager::GetLabel(AddressInfo address, bool checkRegisterLabels) {
    if (DebugUtilities::IsRelativeMemory(address.Type)) {
        if (checkRegisterLabels && InternalGetLabel(address, label)) {
            return label;
        }
        address = _debugger->GetAbsoluteAddress(address);
    }

    return InternalGetLabel(address, label) ? label : "";
}

bool LabelManager::InternalGetLabel(AddressInfo address, std::string &label) {
    int64_t key = GetLabelKey(address.Address, address.Type);
    auto it = _codeLabels.find(key);
    if (it != _codeLabels.end()) {
        label = it->second.Label;
        return true;
    }
    return false;
}

std::string LabelManager::GetComment(AddressInfo absAddress) {
    uint64_t key = GetLabelKey(absAddress.Address, absAddress.Type);
    auto it = _codeLabels.find(key);
    if (it != _codeLabels.end()) {
        return it->second.Comment;
    }
    return "";
}

bool LabelManager::GetLabelAndComment(AddressInfo address, LabelInfo &labelInfo) {
    if (DebugUtilities::IsRelativeMemory(address.Type)) {
        address = _debugger->GetAbsoluteAddress(address);
    }

    int64_t key = GetLabelKey(address.Address, address.Type);
    auto it = _codeLabels.find(key);
    if (it != _codeLabels.end()) {
        labelInfo = it->second;
        return true;
    }
    return false;
}

bool LabelManager::ContainsLabel(std::string &label) {
    return _codeLabelReverseLookup.find(label) != _codeLabelReverseLookup.end();
}

AddressInfo LabelManager::GetLabelAbsoluteAddress(std::string& label) {
    AddressInfo addr{ -1, MemoryType::None };
    auto it = _codeLabelReverseLookup.find(label);
    if (it != _codeLabelReverseLookup.end()) {
        uint64_t key = it->second;
        addr.Type = GetKeyMemoryType(key);
        addr.Address = (int32_t)(key & 0xFFFFFFFF);
    }
    return addr;
}

int32_t LabelManager::GetLabelRelativeAddress(std::string &label, CpuType cpuType) {
    auto it = _codeLabelReverseLookup.find(label);
    if (it == _codeLabelReverseLookup.end()) {
        // Label doesn't exist, try to find a matching multi-byte label
        it = _codeLabelReverseLookup.find(label + "+0");
    }

    if (it != _codeLabelReverseLookup.end()) {
        uint64_t key = it->second;
        MemoryType type = GetKeyMemoryType(key);
        AddressInfo addr{ (int32_t)(key & 0xFFFFFFFF), type };
        if (DebugUtilities::IsRelativeMemory(type)) {
            return addr.Address;
        }
        return _debugger->GetRelativeAddress(addr, cpuType).Address;
    }
    return -2;
}

bool LabelManager::HasLabelOrComment(AddressInfo address) {
    if (DebugUtilities::IsRelativeMemory(address.Type)) {
        address = _debugger->GetAbsoluteAddress(address);
    }

    uint64_t key = GetLabelKey(address.Address, address.Type);
    return _codeLabels.find(key) != _codeLabels.end();
}
