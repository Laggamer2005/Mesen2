#pragma once
#include "pch.h"

class FrozenAddressManager {
private:
    std::unordered_set<uint32_t> _frozenAddresses;

public:
    void UpdateFrozenAddresses(uint32_t start, uint32_t end, bool freeze) {
        if (freeze) {
            _frozenAddresses.insert(start, end + 1);
        } else {
            for (uint32_t i = start; i <= end; i++) {
                _frozenAddresses.erase(i);
            }
        }
    }

    bool IsFrozenAddress(uint32_t addr) const {
        return _frozenAddresses.find(addr) != _frozenAddresses.end();
    }

    void GetFrozenState(uint32_t start, uint32_t end, bool* outState) const {
        for (uint32_t i = start; i <= end; i++) {
            outState[i_-_start] = IsFrozenAddress(i);
        }
    }
};
