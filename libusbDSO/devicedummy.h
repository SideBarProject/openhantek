#pragma once

#include "deviceBase.h"
#include "devicedummy.h"
#include "errorcodes.h"

class DeviceDummy : public DSO::DeviceBase {
public:
    using DeviceBase::DeviceBase;
    virtual void updatePretriggerPosition(double position) {}
    virtual void updateRecordLength(unsigned int index) {}
    virtual void updateSamplerate(DSO::ControlSamplerateLimits *limits, unsigned int downsampler, bool fastRate) {}
    virtual void updateGain(unsigned channel, unsigned char gain, unsigned gainId) {}
    virtual void updateOffset(unsigned int channel, unsigned short int offsetValue) {}
    virtual ErrorCode updateTriggerSource(bool special, unsigned int channel) {return ErrorCode::ERROR_NONE;}
    virtual ErrorCode updateTriggerLevel(unsigned int channel, double level) {return ErrorCode::ERROR_NONE;}
    virtual ErrorCode updateTriggerSlope(DSO::Slope slope) {return ErrorCode::ERROR_NONE;}
};
