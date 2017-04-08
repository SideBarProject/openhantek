////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//
//  Copyright (C) 2008, 2009  Oleg Khudyakov
//  prcoder@potrebitel.ru
//  Copyright (C) 2010 - 2012  Oliver Haag
//  oliver.haag@gmail.com
//
//  This program is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation, either version 3 of the License, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License along with
//  this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////


#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <memory>
#include <climits>
#include <cstring>

#include <libusb-1.0/libusb.h>

#include "protocol.h"
#include "hantekDevice.h"
#include "protocol.h"
#include "utils/timestampDebug.h"
#include "utils/stdStringSplit.h"

namespace Hantek2xxx_5xxx {

bool HantekDevice::runRollmode(std::vector<unsigned char>& data, RollState& rollState,
                               bool& samplingStarted, unsigned& previouslyReadSamples) {
    // Don't iterate through roll mode steps when stopped
    if(!_sampling) {
        return true;
    }

    int errorCode = 0;

    switch(rollState) {
    case RollState::STARTSAMPLING:

        // Sampling hasn't started, update the expected sample count
        previouslyReadSamples = _device->getPacketSize();

        errorCode = sampleThreadBulkCommand(&get<BulkCaptureStart>());
        if(errorCode < 0) {
            if(errorCode == LIBUSB_ERROR_NO_DEVICE)
                return false;
            break;
        }

        timestampDebug("Starting to capture");
        samplingStarted = true;
        break;
    case RollState::ENABLETRIGGER:
        errorCode = sampleThreadBulkCommand(&get<BulkTriggerEnabled>());
        if(errorCode < 0) {
            if(errorCode == LIBUSB_ERROR_NO_DEVICE)
                return false;
            break;
        }
        timestampDebug("Enabling trigger");
        break;
    case RollState::FORCETRIGGER:
        errorCode = sampleThreadBulkCommand(&get<BulkForceTrigger>());
        if(errorCode < 0) {
            if(errorCode == LIBUSB_ERROR_NO_DEVICE)
                return false;
            break;
        }
        timestampDebug("Forcing trigger");
        break;

    case RollState::GETDATA:
        // Get data and process it, if we're still sampling
        errorCode = readSamples(data, _device->getPacketSize(), previouslyReadSamples);
        if(errorCode < 0)
            std::cerr << "Getting sample data failed: " <<
                         libusb_error_name((libusb_error)errorCode) << " " <<
                         libusb_strerror((libusb_error)errorCode) << std::endl;
        else {
            timestampDebug("Received " << errorCode << " B of sampling data");
        }
        // Process the data only if we want it
        if(samplingStarted) {
            processSamples(data);
            _samplesAvailable(_samples);

            // Check if we're in single trigger mode
            if(_settings.trigger.mode == DSO::TriggerMode::SINGLE)
                stopSampling();

            // Sampling completed, restart it when necessary
            samplingStarted = false;
        }

        break;
    default:
        timestampDebug("Roll mode state unknown");
        break;
    }

    // Go to next state, or restart if last state was reached
    rollState = (RollState) (((int)rollState + 1) % (int)RollState::COUNT);

    return true;
}

bool HantekDevice::runStandardMode(std::vector<unsigned char>& data, CaptureState& captureState,
                                   int& cycleCounter, int& startCycle, int timerIntervall,
                                   bool& samplingStarted, DSO::TriggerMode& lastTriggerMode, unsigned& previouslyReadSamples) {
    int errorCode;
    CaptureState lastCaptureState = captureState;
    std::tie(errorCode,captureState) = readCaptureState();

    if(errorCode < 0) {
        std::cerr <<"Getting capture state failed: " <<
                    libusb_error_name((libusb_error)errorCode) <<
                    libusb_strerror((libusb_error)errorCode) << std::endl;
        return false;
    }

    // Errorcode is the trigger point if >= 0.
    _settings.trigger.point = errorCode;

    // Hantek encoding for trigger point: Each set bit inverts all bits with a lower value
    for(unsigned int bitValue = 1; bitValue; bitValue <<= 1)
        if(_settings.trigger.point & bitValue)
            _settings.trigger.point ^= bitValue - 1;

    if(captureState != lastCaptureState) {
        timestampDebug("Capture state changed to " << (int)captureState);
    }
    switch(captureState) {
    case CaptureState::READY:
    case CaptureState::READY2250:
    case CaptureState::READY5200:
        // Get data and process it, if we're still sampling
        errorCode = readSamples(data, getExpectedRecordLength(), previouslyReadSamples);
        if(errorCode < 0)
            std::cerr << "Getting sample data failed: " <<
                         libusb_error_name((libusb_error)errorCode) << " " <<
                         libusb_strerror((libusb_error)errorCode) << std::endl;
        else {
            timestampDebug("Received "<< errorCode << " B of sampling data");
        }
        // Process the data only if we want it
        if(samplingStarted) {
            processSamples(data);
            _samplesAvailable(_samples);
        }

        // Check if we're in single trigger mode
        if(_settings.trigger.mode == DSO::TriggerMode::SINGLE && samplingStarted)
            stopSampling();

        // Sampling completed, restart it when necessary
        samplingStarted = false;

        // Start next capture if necessary by leaving out the break statement
        if(!_sampling)
            break;

    case CaptureState::WAITING:
        // Sampling hasn't started, update the expected sample count
        previouslyReadSamples = getExpectedRecordLength();

        if(samplingStarted && lastTriggerMode == _settings.trigger.mode) {
            ++cycleCounter;

            if(cycleCounter == startCycle) {
                // Buffer refilled completely since start of sampling, enable the trigger now
                errorCode = sampleThreadBulkCommand(&get<BulkTriggerEnabled>());
                if(errorCode < 0) {
                    if(errorCode == LIBUSB_ERROR_NO_DEVICE)
                        return false;
                    break;
                }

                timestampDebug("Enabling trigger");

            }
            else if(cycleCounter >= 8 + startCycle && _settings.trigger.mode == DSO::TriggerMode::AUTO) {
                // Force triggering
                errorCode = sampleThreadBulkCommand(&get<BulkForceTrigger>());
                if(errorCode < 0) {
                    if(errorCode == LIBUSB_ERROR_NO_DEVICE)
                        return false;
                    break;
                }
                timestampDebug("Forcing trigger");
            }

            if(cycleCounter < 20 || cycleCounter < 4000 / timerIntervall)
                break;
        }

        // Start capturing
        errorCode = sampleThreadBulkCommand(&get<BulkCaptureStart>());
        if(errorCode < 0) {
            if(errorCode == LIBUSB_ERROR_NO_DEVICE)
                return false;
            break;
        }

        timestampDebug("Starting to capture");

        samplingStarted = true;
        cycleCounter = 0;
        startCycle = _settings.trigger.pretrigger_pos_in_s * 1000 / timerIntervall + 1;
        lastTriggerMode = _settings.trigger.mode;
        break;

    case CaptureState::SAMPLING:
        break;
    default:
        break;
    }
    return true;
}

void HantekDevice::run() {
    // Initialize usb communication thread state
    CaptureState captureState = CaptureState::WAITING;
    RollState rollState       = RollState::STARTSAMPLING;
    bool samplingStarted      = false;
    DSO::TriggerMode lastTriggerMode = DSO::TriggerMode::UNDEFINED;
    int cycleCounter          = 0;
    int startCycle            = 0;
    unsigned previouslyReadSamples = 0;
    std::vector<unsigned char> data;

    while (_keep_thread_running) {
        if (!sendPendingCommands(_device.get())) break;

        // Compute sleep time
        int cycleTime;

        if(isRollingMode())
            cycleTime = _device->getPacketSize() / (!isFastRate() ? 1 : _specification.channels);
        else
            cycleTime = getCurrentRecordType().length_per_channel;

        cycleTime /= _settings.samplerate.current; // RecLen/Samplerate=Duration in s
        cycleTime *= 1000; // Duration in ms

        // Check the current oscilloscope state, everytime 25% of the time
        // the buffer should be refilled.
        // Not more often than every 10 ms though but at least once every second
        cycleTime = std::min(std::max(10, int(cycleTime * 0.25)), 1000);

        // State machine for the device communication
        if(isRollingMode()) {
            if (!runRollmode(data, rollState, samplingStarted, previouslyReadSamples)) break;
        } else {
            if (!runStandardMode(data, captureState, cycleCounter,
                                 startCycle, cycleTime, samplingStarted, lastTriggerMode, previouslyReadSamples)) break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(cycleTime));
    }

    _device->disconnect();
    _statusMessage(LIBUSB_ERROR_NO_DEVICE);
}

std::pair<int, CaptureState> HantekDevice::readCaptureState() {
    int errorCode = 0;

    errorCode = sampleThreadBulkCommand(&get<BulkGetCaptureState>());
    if(errorCode < 0)
        return std::make_pair(errorCode,CaptureState::UNDEFINED);

    BulkResponseGetCaptureState response;
    errorCode = _device->bulkRead(response.data(), response.size());
    if(errorCode < 0)
        return std::make_pair(errorCode,CaptureState::UNDEFINED);

    return std::make_pair(response.getTriggerPoint(),response.getCaptureState());
}


int HantekDevice::readSamples(std::vector<unsigned char>& data, unsigned recordLength, unsigned& previouslyReadSamples) {
    int errorCode;

    // Request data
    errorCode = sampleThreadBulkCommand(&get<BulkGetData>());
    if(errorCode < 0)
        return errorCode;

    // To make sure no samples will remain in the scope buffer,
    // also check the sample count before the last sampling started
    if(recordLength < previouslyReadSamples)
        std::swap(previouslyReadSamples,recordLength);
    else
        previouslyReadSamples = recordLength;

    if(_specification.sampleSize > 8) // for ADCs with resolution of 9bit..16bit
        recordLength *= 2;            // we need two bytes for the transfer

    data.resize(recordLength);

    errorCode = _device->bulkReadMulti(&data[0], data.size());
    if(errorCode < 0)
        return errorCode;
    recordLength = errorCode; // actual data read
    data.resize(recordLength);

    return errorCode;
}


int HantekDevice::sampleThreadBulkCommand(USBTransferBuffer* command) {
    // Send BeginCommand control command
    ControlBeginCommand& cmd = get<ControlBeginCommand>();
    int errorCode = _device->controlWrite(CONTROL_BEGINCOMMAND, cmd.data(), cmd.size());
    if(errorCode < 0)
        return errorCode;

    return _device->bulkWrite(command->data(), command->size());
}

}
