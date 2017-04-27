#include "devicedummy.h"
#include <iostream>

void DeviceDummy::updatePretriggerPosition(double position) {
    std::cout << "deviceDummy: new pretrigger position: " << position << std::endl;
}
void DeviceDummy::updateRecordLength(unsigned int index) {}

ErrorCode DeviceDummy::updateSamplerate(double timebase) {
    std::cout << "setting sampling rate for timebase: " << timebase << std::endl;
    for (int i=0; i<_specification.availableSamplingRates.size();i++) {
        if (_specification.availableSamplingRates[i].timeBase == timebase)
            std::cout << "Setting sampling rate to " << _specification.availableSamplingRates[i].samplingRateID << std::endl;
    }
    return ErrorCode::ERROR_NONE;
}

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
