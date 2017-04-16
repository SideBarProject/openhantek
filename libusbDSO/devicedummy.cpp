#include "devicedummy.h"
#include <iostream>

void DeviceDummy::updatePretriggerPosition(double position) {
    std::cout << "deviceDummy: new pretrigger position: " << position << std::endl;
}
void DeviceDummy::updateRecordLength(unsigned int index) {}
void DeviceDummy::updateSamplerate(DSO::ControlSamplerateLimits *limits, unsigned int downsampler, bool fastRate) {}
ErrorCode DeviceDummy::updateGain(unsigned channel, unsigned char gain) {
    std::cout << "deviceDummy: updateGain on channel " << channel << " to " << (int)gain  << "(hw gain code)" << std::endl;
    return ErrorCode::ERROR_NONE;
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
