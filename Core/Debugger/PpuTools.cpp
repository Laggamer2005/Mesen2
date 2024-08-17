#include "pch.h"
#include "Debugger/PpuTools.h"
#include "Debugger/CdlManager.h"
#include "Debugger/DebugTypes.h"
#include "Debugger/DebugBreakHelper.h"
#include "Shared/SettingTypes.h"

PpuTools::PpuTools(Debugger* debugger, Emulator* emu)
    : _emu(emu)
    , _debugger(debugger)
{}

void PpuTools::BlendColors(uint8_t output[4], uint8_t input[4])
{
    int alpha = input[3] + 1;
    uint8_t invertedAlpha = 256 - input[3];
    output[0] = static_cast<uint8_t>(((alpha * input[0] + invertedAlpha * output[0]) >> 8));
    output[1] = static_cast<uint8_t>(((alpha * input[1] + invertedAlpha * output[1]) >> 8));
    output[2] = static_cast<uint8_t>(((alpha * input[2] + invertedAlpha * output[2]) >> 8));
    output[3] = 0xFF;
}

void PpuTools::GetTileView(GetTileViewOptions options, uint8_t* source, uint32_t srcSize, const uint32_t* colors, uint32_t* outBuffer)
{
    switch (options.Format) {
        case TileFormat::Bpp2:
        case TileFormat::Bpp4:
        case TileFormat::Bpp8:
        case TileFormat::DirectColor:
        case TileFormat::Mode7:
        case TileFormat::Mode7DirectColor:
        case TileFormat::Mode7ExtBg:
        case TileFormat::NesBpp2:
        case TileFormat::PceSpriteBpp4:
        case TileFormat::PceBackgroundBpp2Cg0:
        case TileFormat::PceBackgroundBpp2Cg1:
        case TileFormat::PceSpriteBpp2Sp01:
        case TileFormat::PceSpriteBpp2Sp23:
        case TileFormat::SmsBpp4:
        case TileFormat::SmsSgBpp1:
        case TileFormat::GbaBpp4:
        case TileFormat::GbaBpp8:
            InternalGetTileView(options, source, srcSize, colors, outBuffer);
            break;
    }
}

uint32_t PpuTools::GetBackgroundColor(TileBackground bgColor, const uint32_t* colors, uint8_t paletteIndex, uint8_t bpp)
{
    switch (bgColor) {
        default:
        case TileBackground::Default:
            return colors[0];
        case TileBackground::PaletteColor:
            return colors[paletteIndex_*_(1_<<_bpp)];
        case TileBackground::Black:
            return 0xFF000000;
        case TileBackground::White:
            return 0xFFFFFFFF;
        case TileBackground::Magenta:
            return 0xFFFF00FF;
        case TileBackground::Transparent:
            return 0;
    }
}

uint32_t PpuTools::GetSpriteBackgroundColor(SpriteBackground bgColor, const uint32_t* colors, bool useDarkerColor)
{
    switch (bgColor) {
        default:
        case SpriteBackground::Gray:
            return useDarkerColor ? 0xFF333333 : 0xFF666666;
        case SpriteBackground::Background:
            return useDarkerColor ? (((colors[0] >> 1) & 0x7F7F7F) | 0xFF000000) : colors[0];
        case SpriteBackground::Black:
            return useDarkerColor ? 0xFF000000 : 0xFF202020;
        case SpriteBackground::White:
            return useDarkerColor ? 0xFFEEEEEE : 0xFFFFFFFF;
        case SpriteBackground::Magenta:
            return useDarkerColor ? 0xFFCC00CC : 0xFFFF00FF;
        case SpriteBackground::Transparent:
            return 0;
    }
}

template <TileFormat format>
void PpuTools::InternalGetTileView(GetTileViewOptions options, uint8_t* source, uint32_t srcSize, const uint32_t* colors, uint32_t* outBuffer)
{
    // ...
}

void PpuTools::SetViewerUpdateTiming(uint32_t viewerId, uint16_t scanline, uint16_t cycle)
{
    DebugBreakHelper helper(_debugger);
    ViewerRefreshConfig cfg;
    cfg.Scanline = scanline;
    cfg.Cycle = cycle;
    _updateTimings[viewerId] = cfg;
}

void PpuTools::RemoveViewer(uint32_t viewerId)
{
    DebugBreakHelper helper(_debugger);
    _updateTimings.erase(viewerId);
}

int32_t PpuTools::GetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y)
{
    int32_t color = 0;
    GetSetTilePixel(tileAddress, format, x, y, color, true);
    return color;
}

void PpuTools::SetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t color)
{
    GetSetTilePixel(tileAddress, format, x, y, color, false);
}

void PpuTools::GetSetTilePixel(AddressInfo tileAddress, TileFormat format, int32_t x, int32_t y, int32_t& color, bool forGet)
{
    // ...
}

void PpuTools::UpdateViewers(uint16_t scanline, uint16_t cycle)
{
    for (auto updateTiming : _updateTimings) {
        ViewerRefreshConfig cfg = updateTiming.second;
        if (cfg.Cycle == cycle && cfg.Scanline == scanline) {
            _emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ViewerRefresh, (void*)(uint64_t)updateTiming.first);
        }
    }
}
