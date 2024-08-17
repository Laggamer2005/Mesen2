
void BaseEventManager::FilterEvents()
{
    auto lock = _lock.AcquireSafe();
    _sentEvents.clear();

    if (ShowPreviousFrameEvents() && !_forAutoRefresh) {
        int offset = GetScanlineOffset();
        uint32_t key = (_snapshotScanline << 16) + _snapshotCycle;
        for (auto& evt : _snapshotPrevFrame) {
            uint32_t evtKey = ((evt.Scanline + offset) << 16) + evt.Cycle;
            if (evtKey > key) {
                EventViewerCategoryCfg eventCfg = GetEventConfig(evt);
                if (eventCfg.Visible) {
                    evt.Flags |= static_cast<uint32_t>(EventFlags::PreviousFrame);
                    _sentEvents.push_back(evt);
                }
            }
        }
    }

    for (auto& evt : _snapshotCurrentFrame) {
        EventViewerCategoryCfg eventCfg = GetEventConfig(evt);
        if (eventCfg.Visible) {
            _sentEvents.push_back(evt);
        }
    }
}

void BaseEventManager::DrawDot(uint32_t x, uint32_t y, uint32_t color, bool drawBackground, uint32_t* buffer)
{
    FrameInfo size = GetDisplayBufferSize();

    if (drawBackground) {
        color = 0xFF000000 | ((color >> 1) & 0x7F7F7F);
    } else {
        color |= 0xFF000000;
    }

    int iMin = drawBackground ? -2 : 0;
    int iMax = drawBackground ? 3 : 1;
    int jMin = drawBackground ? -2 : 0;
    int jMax = drawBackground ? 3 : 1;

    for (int i = iMin; i <= iMax; i++) {
        for (int j = jMin; j <= jMax; j++) {
            int pos = (y + i) * size.Width + x + j;
            if (pos >= 0 && pos < size.Width * size.Height) {
                buffer[pos] = color;
            }
        }
    }
}

void BaseEventManager::GetEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount)
{
    auto lock = _lock.AcquireSafe();
    uint32_t eventCount = std::min(maxEventCount, static_cast<uint32_t>(_sentEvents.size()));
    for (uint32_t i = 0; i < eventCount; i++) {
        _sentEvents[i].Color = GetEventConfig(_sentEvents[i]).Color;
    }
    std::copy_n(_sentEvents.data(), eventCount, eventArray);
    maxEventCount = eventCount;
}

uint32_t BaseEventManager::GetEventCount()
{
    auto lock = _lock.AcquireSafe();
    FilterEvents();
    return static_cast<uint32_t>(_sentEvents.size());
}

void BaseEventManager::ClearFrameEvents()
{
    auto lock = _lock.AcquireSafe();
    _prevDebugEvents = _debugEvents;
    _debugEvents.clear();
}

void BaseEventManager::GetDisplayBuffer(uint32_t* buffer, uint32_t bufferSize)
{
    auto lock = _lock.AcquireSafe();
    FrameInfo size = GetDisplayBufferSize();
    if (_snapshotScanline < 0 || bufferSize < size.Width * size.Height * sizeof(uint32_t)) {
        return;
    }

    std::fill_n(buffer, size.Width * size.Height, 0xFF555555);
    DrawScreen(buffer);
    DrawEvents(buffer, size);
}

void BaseEventManager::DrawLine(uint32_t* buffer, FrameInfo size, uint32_t color, uint32_t row)
{
    int32_t x = 0;
    int32_t y = row - GetScanlineOffset();
    ConvertScanlineCycleToRowColumn(x, y);
    uint32_t offset = y * size.Width;
    std::fill_n(buffer + offset, size.Width, color);
    std::fill_n(buffer + offset + size.Width, size.Width, color);
}

void BaseEventManager::DrawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer)
{
    EventViewerCategoryCfg evtCfg = GetEventConfig(evt);
    uint32_t color = evtCfg.Color;

    int32_t y = evt.Scanline;
    int32_t x = evt.Cycle;
    ConvertScanlineCycleToRowColumn(x, y);

    DrawDot(x, y, color, drawBackground, buffer);
}

void BaseEventManager::DrawEvents(uint32_t* buffer, FrameInfo size)
{
    if (!_forAutoRefresh) {
        DrawLine(buffer, size, 0xFFFFFF55, _snapshotScanline);
    }

    FilterEvents();
    for (auto& evt : _sentEvents) {
        DrawEvent(evt, true, buffer);
    }
    for (auto& evt : _sentEvents) {
        DrawEvent(evt, false, buffer);
    }

    if (!_forAutoRefresh) {
        int32_t y = _snapshotScanline + _snapshotScanlineOffset;
        int32_t x = _snapshotCycle;
        ConvertScanlineCycleToRowColumn(x, y);

        DrawDot(x, y, 0xFF990099, true, buffer);
        DrawDot(x, y, 0xFFFF00FF, false, buffer);
    }
}
