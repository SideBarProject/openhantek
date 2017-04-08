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

#include <vector>
#include <iostream>
#include <memory>

#include <libusb-1.0/libusb.h>

#include "protocol.h"
#include "hantekDevice.h"
#include "protocol.h"
#include "utils/timestampDebug.h"
#include "utils/stdStringSplit.h"

namespace Hantek2xxx_5xxx {

HantekDevice::HantekDevice(std::unique_ptr<DSO::USBCommunication> device)
    : DeviceBase(device->model()), _device(std::move(device)) {
    _device->setDisconnected_signal(std::bind( &HantekDevice::deviceDisconnected, this));
}

HantekDevice::~HantekDevice() {
    _device->disconnect();
}

unsigned HantekDevice::getUniqueID() const {
    return _device->getUniqueID();
}

bool HantekDevice::needFirmware() const {
    return false;
}

ErrorCode HantekDevice::uploadFirmware() {
    //TODO: Store firmware files together with the software
    return ErrorCode::ERROR_NONE;
}

void HantekDevice::deviceDisconnected() {
    if (!_thread.get()) return;
    _keep_thread_running = false;
    if (_thread->joinable()) _thread->join();
    _thread.reset();
}

void HantekDevice::disconnectDevice() {
    _device->disconnect();
}

bool HantekDevice::isDeviceConnected() const {
    return _device->isConnected();
}

ErrorCode HantekDevice::setChannelUsed(unsigned int channel, bool used) {
    DeviceBase::setChannelUsed(channel, used);

    // Calculate the UsedChannels field for the command
    unsigned char usedChannels = USED_CH1;

    if(_settings.voltage[1].used) {
        if(_settings.voltage[0].used) {
            usedChannels = USED_CH1CH2;
        }
        else {
            // DSO-2250 uses a different value for channel 2
            if(_model.productID == 0x2250)
                usedChannels = BUSED_CH2;
            else
                usedChannels = USED_CH2;
        }
    }


    switch(_model.productID) {
        case 0x2150:
        case 0x2090: {
            addPending(get<BulkSetTriggerAndSamplerate>().setUsedChannels(usedChannels));
            break;
        }
        case 0x2250: {
            addPending(get<BulkSetChannels2250>().setUsedChannels(usedChannels));
            break;
        }
        case 0x520A:
        case 0x5200: {
            addPending(get<BulkSetTrigger5200>().setUsedChannels(usedChannels));
            break;
        }
        default:
            break;
    }

    return ErrorCode::ERROR_NONE;
}

ErrorCode HantekDevice::setCoupling(unsigned int channel, DSO::Coupling coupling) {
    if(channel >= _specification.channels)
        return ErrorCode::ERROR_PARAMETER;

    // SetRelays control command for coupling relays
    ControlSetRelays& cmd = get<ControlSetRelays>();
    cmd.setCoupling(channel, coupling != DSO::Coupling::AC);
    addPending(cmd);

    return ErrorCode::ERROR_NONE;
}

void HantekDevice::updateGain(unsigned channel, unsigned char gainIndex, unsigned gainId)
{
    // SetGain bulk command for gain
    addPending(get<BulkSetGain>().setGain(channel, gainIndex));

    // SetRelays control command for gain relays
    ControlSetRelays& control = get<ControlSetRelays>();
    control.setBelow1V(channel, gainId < 3);
    control.setBelow100mV(channel, gainId < 6);
    addPending(control);
}

void HantekDevice::updateOffset(unsigned int channel, unsigned short offsetValue)
{
    addPending(get<ControlSetOffset>().setChannel(channel, offsetValue));
}

ErrorCode HantekDevice::updateTriggerSource(bool special, unsigned int id) {
    if((!special && id >= _specification.channels) || (special && id >= _specification.channels_special))
        return ErrorCode::ERROR_PARAMETER;

    switch(_model.productID) {
        case 0x2150:
        case 0x2090: {
            // SetTriggerAndSamplerate bulk command for trigger source
            BulkSetTriggerAndSamplerate& cmd = get<BulkSetTriggerAndSamplerate>();
            cmd.setTriggerSource(special ? 3 + id : 1 - id);
            addPending(cmd);
            break;
        }
        case 0x2250: {
            // SetTrigger2250 bulk command for trigger source
            BulkSetTrigger2250& cmd = get<BulkSetTrigger2250>();
            cmd.setTriggerSource(special ? 0 : 2 + id);
            addPending(cmd);
            break;
        }
        case 0x520A:
        case 0x5200: {
            // SetTrigger5200 bulk command for trigger source
            BulkSetTrigger5200& cmd = get<BulkSetTrigger5200>();
            cmd.setTriggerSource(special ? 3 + id : 1 - id);
            addPending(cmd);
            break;
        }
        default:
            return ErrorCode::ERROR_UNSUPPORTED;
    }

    // SetRelays control command for external trigger relay
    addPending(get<ControlSetRelays>().setTrigger(special));

    // Apply trigger level of the new source
    if(special) {
        // SetOffset control command for changed trigger level
        addPending(get<ControlSetOffset>().setTrigger(0x7f));
    }

    return ErrorCode::ERROR_NONE;
}


ErrorCode HantekDevice::updateTriggerLevel(unsigned int channel, double level) {
    if(channel >= _specification.channels)
        return ErrorCode::ERROR_PARAMETER;

    if(_settings.trigger.special || channel != _settings.trigger.source)
        return ErrorCode::ERROR_PARAMETER;

    // Calculate the trigger level value
    unsigned short int minimum, maximum;
    switch(_model.productID) {
        case 0x5200:
        case 0x520A:
            // The range is the same as used for the offsets for 10 bit models
            minimum = getGainLevel(channel).offset[channel].minimum;
            maximum = getGainLevel(channel).offset[channel].maximum;
            break;

        default:
            // It's from 0x00 to 0xfd for the 8 bit models
            minimum = 0x00;
            maximum = 0xfd;
            break;
    }

    // Never get out of the limits
    unsigned short int tlevel = _settings.voltage[channel].offsetReal + level / getGainLevel(channel).gainSteps * (maximum - minimum) + 0.5  + minimum;
    unsigned short int levelValue = std::max(std::min(minimum, tlevel), maximum);

    // SetOffset control command for trigger level
    addPending(get<ControlSetOffset>().setTrigger(levelValue));

    /// \todo Get alternating trigger in here

    // ((levelValue - minimum) / (maximum - minimum) - _settings.voltage[channel].offsetReal) * _specification.gainSteps[_settings.voltage[channel].gain];
    return ErrorCode::ERROR_NONE;
}

ErrorCode HantekDevice::updateTriggerSlope(DSO::Slope slope) {
    switch(_model.productID) {
        case 0x2150:
        case 0x2090: {
            addPending(get<BulkSetTriggerAndSamplerate>().setTriggerSlope((uint8_t)slope));
            break;
        }
        case 0x2250: {
            addPending(get<BulkSetTrigger2250>().setTriggerSlope((uint8_t)slope));
            break;
        }
        case 0x520A:
        case 0x5200: {
            addPending(get<BulkSetTrigger5200>().setTriggerSlope((uint8_t)slope));
            break;
        }
        default:
            throw std::runtime_error("model not supported");
    }

    return ErrorCode::ERROR_NONE;
}

void HantekDevice::updatePretriggerPosition(double pretrigger_pos_in_s) {
    // All trigger positions are measured in samples
    unsigned int positionSamples = pretrigger_pos_in_s * _settings.samplerate.current;
    unsigned int recordLength = getCurrentRecordType().length_per_channel;
    bool rollMode = recordLength == rollModeValue;
    // Fast rate mode uses both channels
    if(isFastRate())
        positionSamples /= _specification.channels;

    switch(_model.productID) {
        case 0x2150:
        case 0x2090: {
            // Calculate the position value (Start point depending on record length)
            unsigned int position = rollMode ? 0x1 : 0x7ffff - recordLength + positionSamples;
            addPending(get<BulkSetTriggerAndSamplerate>().setTriggerPosition(position));
            break;
        }
        case 0x2250: {
            // Calculate the position values (Inverse, maximum is 0x7ffff)
            unsigned int positionPre = 0x7ffff - recordLength + positionSamples;
            unsigned int positionPost = 0x7ffff - positionSamples;

            BulkSetBuffer2250& cmd = get<BulkSetBuffer2250>();
            cmd.setTriggerPositionPre(positionPre);
            cmd.setTriggerPositionPost(positionPost);
            addPending(cmd);

            break;
        }
        case 0x520A:
        case 0x5200: {
            // Calculate the position values (Inverse, maximum is 0xffff)
            unsigned short int positionPre = 0xffff - recordLength + positionSamples;
            unsigned short int positionPost = 0xffff - positionSamples;

            BulkSetBuffer5200& cmd = get<BulkSetBuffer5200>();
            cmd.setTriggerPositionPre(positionPre);
            cmd.setTriggerPositionPost(positionPost);
            addPending(cmd);

            break;
        }
        default:
            throw std::runtime_error("model not supported");
    }
}


void HantekDevice::updateRecordLength(unsigned int index) {
    if(index >= (unsigned int) getRecordTypes().size())
        throw std::range_error("updateRecordLength index out of range");

    switch(_model.productID) {
        case 0x2150:
        case 0x2090: {
            // SetTriggerAndSamplerate bulk command for record length
            BulkSetTriggerAndSamplerate& cmd = get<BulkSetTriggerAndSamplerate>();
            cmd.setRecordLength(index);
            addPending(cmd);
            break;
        }
        case 0x2250: {
            // Pointers to needed commands
            BulkSetRecordLength2250& cmd = get<BulkSetRecordLength2250>();
            cmd.setRecordLength(index);
            addPending(cmd);
            break;
        }
        case 0x520A:
        case 0x5200: {
            // SetBuffer5200 bulk command for record length
            BulkSetBuffer5200& cmd = get<BulkSetBuffer5200>();
            cmd.setRecordLength(index);
            cmd.setUsedPre(DTRIGGERPOSITION_ON);
            cmd.setUsedPost(DTRIGGERPOSITION_ON);
            addPending(cmd);
            break;
        }
        default:
            throw std::runtime_error("model not supported");
    }
}

void HantekDevice::updateSamplerate(DSO::ControlSamplerateLimits *limits, unsigned int downsampler, bool fastRate) {
    // Set the calculated samplerate
    switch(_model.productID) {
        case 0x2150:
        case 0x2090: {
            short int downsamplerValue = 0;
            unsigned char samplerateId = 0;
            bool downsampling = false;

            if(downsampler <= 5) {
                // All dividers up to 5 are done using the special samplerate IDs
                if(downsampler == 0 && limits->base >= limits->max)
                    samplerateId = 1;
                else if(downsampler <= 2)
                    samplerateId = downsampler;
                else { // Downsampling factors 3 and 4 are not supported
                    samplerateId = 3;
                    downsampler = 5;
                    downsamplerValue = 0xffff;
                }
            }
            else {
                // For any dividers above the downsampling factor can be set directly
                downsampler &= ~0x0001; // Only even values possible
                downsamplerValue = (short int) (0x10001 - (downsampler >> 1));

                downsampling = true;
            }

            // Pointers to needed commands
            BulkSetTriggerAndSamplerate& cmd = get<BulkSetTriggerAndSamplerate>();

            // Store if samplerate ID or downsampling factor is used
            cmd.setDownsamplingMode(downsampling);
            // Store samplerate ID
            cmd.setSamplerateId(samplerateId);
            // Store downsampling factor
            cmd.setDownsampler(downsamplerValue);
            // Set fast rate when used
            cmd.setFastRate(false /*fastRate*/);

            addPending(cmd);

            break;
        }
        case 0x520A:
        case 0x5200: {
            // Split the resulting divider into the values understood by the device
            // The fast value is kept at 4 (or 3) for slow sample rates
            long int valueSlow = std::max(((long int) downsampler - 3) / 2, (long int) 0);
            unsigned char valueFast = downsampler - valueSlow * 2;

            BulkSetSamplerate5200& cmdSamplerate = get<BulkSetSamplerate5200>();
            BulkSetTrigger5200& cmdSettrigger = get<BulkSetTrigger5200>();

            // Store samplerate fast value
            cmdSamplerate.setSamplerateFast(4 - valueFast);
            // Store samplerate slow value (two's complement)
            cmdSamplerate.setSamplerateSlow(valueSlow == 0 ? 0 : 0xffff - valueSlow);
            // Set fast rate when used
            cmdSettrigger.setFastRate(fastRate);

            addPending(cmdSamplerate);
            addPending(cmdSettrigger);

            break;
        }
        case 0x2250: {
            // Pointers to needed commands
            BulkSetSamplerate2250& cmd = get<BulkSetSamplerate2250>();

            bool downsampling = downsampler >= 1;
            // Store downsampler state value
            cmd.setDownsampling(downsampling);
            // Store samplerate value
            cmd.setSamplerate(downsampler > 1 ? 0x10001 - downsampler : 0);
            // Set fast rate when used
            cmd.setFastRate(fastRate);

            addPending(cmd);

            break;
        }
        default:
            throw std::runtime_error("model not supported");
    }
}

double HantekDevice::getDownsamplerRate(double bestDownsampler, bool maximum) const
{
    switch(_model.productID) {
        case 0x2150:
        case 0x2090:
            // DSO-2090 supports the downsampling factors 1, 2, 4 and 5 using valueFast
            // or all even values above using valueSlow
            if((maximum && bestDownsampler <= 5.0) || (!maximum && bestDownsampler < 6.0)) {
                // valueFast is used
                if(maximum) {
                    // The samplerate shall not be higher, so we round up
                    bestDownsampler = ceil(bestDownsampler);
                    if(bestDownsampler > 2.0) // 3 and 4 not possible with the DSO-2090
                        bestDownsampler = 5.0;
                }
                else {
                    // The samplerate shall not be lower, so we round down
                    bestDownsampler = floor(bestDownsampler);
                    if(bestDownsampler > 2.0 && bestDownsampler < 5.0) // 3 and 4 not possible with the DSO-2090
                        bestDownsampler = 2.0;
                }
            }
            else {
                // valueSlow is used
                if(maximum) {
                    bestDownsampler = ceil(bestDownsampler / 2.0) * 2.0; // Round up to next even value
                }
                else {
                    bestDownsampler = floor(bestDownsampler / 2.0) * 2.0; // Round down to next even value
                }
                if(bestDownsampler > 2.0 * 0x10001) // Check for overflow
                    bestDownsampler = 2.0 * 0x10001;
            }
            break;

        case 0x520A:
        case 0x5200:
        case 0x2250:
        // DSO-2250 doesn't have a fast value, so it supports all downsampling factors
        // DSO-5200 may not supports all downsampling factors, requires testing
            if(maximum) {
                bestDownsampler = ceil(bestDownsampler); // Round up to next integer value
            }
            else {
                bestDownsampler = floor(bestDownsampler); // Round down to next integer value
            }
            break;
        default:
            throw std::runtime_error("model not supported");
    }
    return bestDownsampler;
}

}
