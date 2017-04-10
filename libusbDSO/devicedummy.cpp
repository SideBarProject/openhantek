#include "devicedummy.h"
#include <iostream>

void DeviceDummy::updatePretriggerPosition(double position) {
    std::cout << "deviceDummy: new pretrigger position: " << position << std::endl;
}
void DeviceDummy::updateRecordLength(unsigned int index) {}
void DeviceDummy::updateSamplerate(DSO::ControlSamplerateLimits *limits, unsigned int downsampler, bool fastRate) {}
void DeviceDummy::updateGain(unsigned channel, unsigned char gain, unsigned gainId) {
    std::cout << "deviceDummy: updateGain on channel " << channel << "to " << gain << std::endl;
}
void DeviceDummy::updateOffset(unsigned int channel, unsigned short int offsetValue) {}

ErrorCode DeviceDummy::updateTriggerSource(bool special, unsigned int channel) {
    return ErrorCode::ERROR_NONE;
}
ErrorCode DeviceDummy::updateTriggerLevel(unsigned int channel, double level) {
    return ErrorCode::ERROR_NONE;
}
ErrorCode DeviceDummy::updateTriggerSlope(DSO::Slope slope) {
    return ErrorCode::ERROR_NONE;
}
