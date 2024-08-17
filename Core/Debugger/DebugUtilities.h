#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Shared/MemoryType.h"
#include "Utilities/HexUtilities.h"

class DebugUtilities
{
public:
    static constexpr MemoryType GetCpuMemoryType(CpuType type)
    {
        static constexpr std::array<MemoryType, 13> cpuMemoryMap = {
            MemoryType::SnesMemory,  // Snes
            MemoryType::SpcMemory,  // Spc
            MemoryType::NecDspMemory,  // NecDsp
            MemoryType::Sa1Memory,  // Sa1
            MemoryType::GsuMemory,  // Gsu
            MemoryType::Cx4Memory,  // Cx4
            MemoryType::GameboyMemory,  // Gameboy
            MemoryType::NesMemory,  // Nes
            MemoryType::PceMemory,  // Pce
            MemoryType::SmsMemory,  // Sms
            MemoryType::GbaMemory,  // Gba
            MemoryType::None,  // Invalid
            MemoryType::None   // Invalid
        };

        return cpuMemoryMap[static_cast<int>(type)];
    }

    static constexpr int GetProgramCounterSize(CpuType type)
    {
        static constexpr std::array<int, 11> programCounterSizeMap = {
            6,  // Snes
            4,  // Spc
            6,  // NecDsp
            6,  // Sa1
            6,  // Gsu
            6,  // Cx4
            4,  // Gameboy
            4,  // Nes
            4,  // Pce
            4,  // Sms
            8   // Gba
        };

        return programCounterSizeMap[static_cast<int>(type)];
    }

    static constexpr CpuType ToCpuType(MemoryType type)
    {
        static constexpr std::array<CpuType, 35> cpuTypeMap = {
            CpuType::Snes,  // SnesMemory
            // ...
            CpuType::Gba  // GbaMemory
        };

        return cpuTypeMap[static_cast<int>(type)];
    }

    // ...
};
