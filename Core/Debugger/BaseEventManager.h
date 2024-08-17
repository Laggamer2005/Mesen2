#pragma once
#include "pch.h"
#include "Debugger/DebugTypes.h"
#include "Shared/SettingTypes.h"
#include "Utilities/SimpleLock.h"
#include "SNES/DmaControllerTypes.h"

enum class EventFlags : uint32_t {
    PreviousFrame = 1,
    RegFirstWrite = 2,
    RegSecondWrite = 4,
    WithTargetMemory = 8,
    SmsVdpPaletteWrite = 16
};

struct DebugEventInfo {
    MemoryOperationInfo operation;
    DebugEventType type;
    uint32_t programCounter;
    int16_t scanline;
    int16_t cycle;
    int16_t breakpointId = -1;
    int8_t dmaChannel = -1;
    DmaChannelConfig dmaChannelInfo;
    uint32_t flags;
    int32_t registerId = -1;
    MemoryOperationInfo targetMemory;
    uint32_t color = 0;
};

struct EventViewerCategoryCfg {
    bool visible;
    uint32_t color;
};

struct BaseEventViewerConfig {};

class BaseEventManager {
protected:
    std::vector<DebugEventInfo> debugEvents;
    std::vector<DebugEventInfo> prevDebugEvents;
    std::vector<DebugEventInfo> sentEvents;

    std::vector<DebugEventInfo> snapshotCurrentFrame;
    std::vector<DebugEventInfo> snapshotPrevFrame;
    int16_t snapshotScanline = -1;
    int16_t snapshotScanlineOffset = 0;
    uint16_t snapshotCycle = 0;
    bool forAutoRefresh = false;
    SimpleLock lock;

    virtual bool showPreviousFrameEvents() = 0;

    void filterEvents();
    void drawDot(uint32_t x, uint32_t y, uint32_t color, bool drawBackground, uint32_t* buffer);
    virtual int getScanlineOffset() { return 0; }

    void drawLine(uint32_t* buffer, FrameInfo size, uint32_t color, uint32_t row);
    void drawEvents(uint32_t* buffer, FrameInfo size);

    virtual void convertScanlineCycleToRowColumn(int32_t& x, int32_t& y) = 0;
    virtual void drawScreen(uint32_t* buffer) = 0;
    void drawEvent(DebugEventInfo& evt, bool drawBackground, uint32_t* buffer);

public:
    virtual ~BaseEventManager() {}

    virtual void setConfiguration(BaseEventViewerConfig& config) = 0;

    virtual void addEvent(DebugEventType type, MemoryOperationInfo& operation, int32_t breakpointId = -1) = 0;
    virtual void addEvent(DebugEventType type) = 0;

    void getEvents(DebugEventInfo* eventArray, uint32_t& maxEventCount);
    uint32_t getEventCount();
    virtual void clearFrameEvents();

    virtual EventViewerCategoryCfg getEventConfig(DebugEventInfo& evt) = 0;

    virtual uint32_t takeEventSnapshot(bool forAutoRefresh) = 0;
    virtual FrameInfo getDisplayBufferSize() = 0;
    virtual DebugEventInfo getEvent(uint16_t scanline, uint16_t cycle) = 0;

    void getDisplayBuffer(uint32_t* buffer, uint32_t bufferSize);
};
