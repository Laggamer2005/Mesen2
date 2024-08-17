#pragma once

// Forward declaration of MemoryType
enum class MemoryType;

// Struct to hold address information
struct AddressInfo {
    int32_t address;  // Conventional naming convention uses camelCase
    MemoryType type;
};
