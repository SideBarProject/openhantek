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

#include "deviceBase.h"
#include "dsoSettings.h"

#include <iostream>
#include <math.h>

namespace DSO {
    void DeviceBase::resetSettings()
    {
        _samples.resize(_specification.channels);
//        _settings.samplerate.limits = &(_specification.samplerate_single);
        _specification.gainLevel.clear();

        for(DSO::dsoSettingsChannel& c: _settings.voltage)
            c = DSO::dsoSettingsChannel();

        for(double& c: _settings.trigger.level)
            c = 0.0;

        // clear
        _specification.samplerate_single.recordTypes.clear();
        _specification.samplerate_multi.recordTypes.clear();

        _specification.specialTriggerSources.clear();

    }

    const std::vector<DSO::Coupling> &DeviceBase::getAvailableCoupling() const {
        return _specification.availableCoupling;
    }

    unsigned int DeviceBase::getPhysicalChannels(){
        return _specification.channels;
    }

    double DeviceBase::getSamplingrateFromTimebase(double timebase) {
        const std::vector<DSO::dsoAvailableSamplingRate> &availableSamplingRates=getAvailableSamplingrates();
        for (unsigned int i=0;i<availableSamplingRates.size();i++)
            if (fabs(availableSamplingRates[i].timeBase - timebase) < 1e-9)
                return availableSamplingRates[i].samplingrateValue;
        return -1;
    }

    HWRecordLengthID DeviceBase::getRecordLengthFromTimebase(double timebase) {
        const std::vector<DSO::dsoAvailableSamplingRate> &availableSamplingRates=getAvailableSamplingrates();
        for (unsigned int i=0;i<availableSamplingRates.size();i++)
            if (fabs(availableSamplingRates[i].timeBase - timebase) < 1e-9)
                return availableSamplingRates[i].recordLengthID;
        return RECORDLENGTH_INVALID;
    }

    HWSamplingRateID DeviceBase::getSamplingrateIDFromTimebase(double timebase) {
        const std::vector<DSO::dsoAvailableSamplingRate> &availableSamplingRates=getAvailableSamplingrates();
        for (unsigned int i=0;i<availableSamplingRates.size();i++)
            if (fabs(availableSamplingRates[i].timeBase - timebase) < 1e-9) {
                std::cout << "DeviceBase::getSamplingrateIDFromTimebase: index: " << i << " sampling rate id: " << availableSamplingRates[i].samplingRateID << std::endl;
                return availableSamplingRates[i].samplingRateID;
            }
        return SAMPLINGRATE_INVALID;
    }

    int DeviceBase::getDownsamplerRateFromTimebase(double timebase) {
        const std::vector<DSO::dsoAvailableSamplingRate> &availableSamplingRates=getAvailableSamplingrates();
        for (unsigned int i=0;i<availableSamplingRates.size();i++)
            if (fabs(availableSamplingRates[i].timeBase - timebase) < 1e-9)
                return availableSamplingRates[i].downsampling;
        return -1;
    }

    const std::vector<DSO::HWRecordLengthID> &DeviceBase::getRecordLength() const {
//          return nullptr;
        std::cout << "trying to return record lengths" << std::endl;
        return _specification.availableRecordLengths;
}

    const std::vector<dsoGainLevel> &DeviceBase::getGainSpecs() const {
//          return nullptr;
        std::cout << "trying to return gain specs " << std::endl;
        return _specification.gainLevel;
    }

    /// \brief Get a list of the names of the special trigger sources.
    const std::vector<std::string>& DeviceBase::getSpecialTriggerSources() const {
        return _specification.specialTriggerSources;
    }
    const std::vector<DSO::dsoAvailableSamplingRate> &DeviceBase::getAvailableSamplingrates() const {
        return _specification.availableSamplingRates;
    }

    ErrorCode DeviceBase::setChannelUsed(unsigned int channel, bool used)
    {
        if(channel >= _specification.channels)
            return ErrorCode::ERROR_PARAMETER;

        // Update _settings
        _settings.voltage[channel].used = used;
        unsigned int channelCount = 0;
        for(unsigned channelCounter = 0; channelCounter < _specification.channels; ++channelCounter) {
            if(_settings.voltage[channelCounter].used)
                ++channelCount;
        }

        // Check if fast rate mode availability changed
/*
        bool fastRateChanged = (_settings.usedChannels <= 1) != (channelCount <= 1);
        _settings.usedChannels = channelCount;

        if(fastRateChanged)
            notifySamplerateLimitsChanged();
*/
        return ErrorCode::ERROR_PARAMETER;
    }

    ErrorCode DeviceBase::setCoupling(unsigned int channel, Coupling coupling)
    {
        _settings.voltage[channel].coupling = coupling;
        return ErrorCode::ERROR_PARAMETER;
    }

    ErrorCode DeviceBase::setTimebase(double timebase) {
        _settings.timebase = timebase;
        /* after setting the timebase, update record length, sampling rate and downsampler as well */
        const std::vector<DSO::dsoAvailableSamplingRate> &availableSamplingRates=getAvailableSamplingrates();
        for (unsigned int i=0;i<availableSamplingRates.size();i++)
            if (fabs(availableSamplingRates[i].timeBase - timebase) < 1e-9) {
                _settings.samplerate.downsampler = availableSamplingRates[i].downsampling;
                _settings.samplerate.current = availableSamplingRates[i].samplingrateValue;
                _settings.samplerate.recordLengthID = availableSamplingRates[i].recordLengthID;
            }
        HWSamplingRateID HWsamplingRate = getSamplingrateIDFromTimebase(timebase);
        std::cout <<  "DeviceBase::setTimebase: timebase: " << timebase << " HW sampling rate value: " << HWsamplingRate << std::endl;
        return updateSamplerate(timebase);
    }

    double DeviceBase::getTimebase() {
        return _settings.timebase;
    }

    void DeviceBase::setFirmwareFilename(std::string filename) {
        std::cout << "filename set to " << filename << std::endl;
        _settings.firmwareFilename = filename;
    }

    double DeviceBase::getGainIndex(unsigned int channel) {
        double gain = _settings.voltage[channel].gain*DIVS_VOLTAGE;
        double gainID;
        for (int i=0;i<_specification.gainLevel.size();++i) {
 //           std::cout << "DeviceBase::getGain: gain "  << _specification.gainLevel[i].gainSteps << " compare to " << gain<< std::endl;
            if (_specification.gainLevel[i].gainSteps == gain) {
                gainID = _specification.gainLevel[i].gainIndex;
                break;
            }
        }
        return gainID;
    }

    ErrorCode DeviceBase::setGain(unsigned int channel, double gain)
    {
//        std::cout <<"DeviceBase::setGain on channel: " << channel << " to " << gain << " no of channels " << _specification.channels << std::endl;
        if(!isDeviceConnected())
            return ErrorCode::ERROR_CONNECTION;

        if(channel >= _specification.channels+1)           // 2 physical channels + math channel
            return ErrorCode::ERROR_PARAMETER;

        _settings.voltage[channel].gain = gain/DIVS_VOLTAGE;
        char hwGainCode = -1;
        double gainID;
        for (int i=0;i<_specification.gainLevel.size();++i) {
//            std::cout << "DeviceBase::setGain: gain "  << _specification.gainLevel[i].gainSteps << " compare to " << gain<< std::endl;
            if (_specification.gainLevel[i].gainSteps == gain) {
                hwGainCode = _specification.gainLevel[i].hwGain;
                gainID = _specification.gainLevel[i].gainIndex;
                break;
            }
        }
        if (hwGainCode == -1)
            std::cout << "DeviceBase::setGain: gain " << gain << " not found in specs" << std::endl;
        else {
            _settings.voltage[channel].gainID = gainID;
//            std::cout << "setting gain to " << _settings.voltage[channel].gainID << " hw gain code: " << (int) hwGainCode <<std::endl;
        }
        if (updateGain(channel, hwGainCode) != ErrorCode::ERROR_NONE)
            std::cout << "Error when setting gain" << std::endl;

        return ErrorCode::ERROR_NONE;
    }

    ErrorCode DeviceBase::setOffset(unsigned int channel, double offset)
    {
        if(!isDeviceConnected())
            return ErrorCode::ERROR_CONNECTION;

        if(channel >= _specification.channels)
            return ErrorCode::ERROR_PARAMETER;

        // Calculate the offset value
        // The range is given by the calibration data (convert from big endian)
        unsigned int gainID = _settings.voltage[channel].gainID;
        unsigned short int minimum, maximum, offsetDiff, offsetValue;
        minimum = _specification.gainLevel[gainID].offset[channel].minimum;
        maximum = _specification.gainLevel[gainID].offset[channel].maximum;
        offsetDiff = maximum - minimum;
        offsetValue = offset * offsetDiff + minimum + 0.5;
        double offsetReal = (double) offset + 0.5 / offsetDiff;

        updateOffset(channel, offsetValue);

        _settings.voltage[channel].offset = offset;
        _settings.voltage[channel].offsetReal = offsetReal;

        updateTriggerLevel(channel, _settings.trigger.level[channel]);

        // offsetReal;
        return ErrorCode::ERROR_NONE;
    }

    ErrorCode DeviceBase::setTriggerSource(bool special, unsigned int channel)
    {
        ErrorCode c = updateTriggerSource(special, channel);
        if (c != ErrorCode::ERROR_NONE) return c;

        _settings.trigger.special = special;
        _settings.trigger.source = channel;

        if(!special)
            this->setTriggerLevel(channel, _settings.trigger.level[channel]);

        return ErrorCode::ERROR_NONE;
    }

    ErrorCode DeviceBase::setTriggerLevel(unsigned int channel, double level)
    {
        ErrorCode c = updateTriggerLevel(channel, level);
        if (c != ErrorCode::ERROR_NONE) return c;

        _settings.trigger.level[channel] = level;
        return ErrorCode::ERROR_NONE;
    }

    ErrorCode DeviceBase::setTriggerSlope(Slope slope)
    {
        ErrorCode c = updateTriggerSlope(slope);
        if (c != ErrorCode::ERROR_NONE) return c;

        _settings.trigger.slope = slope;
        return ErrorCode::ERROR_NONE;
    }

    ErrorCode DeviceBase::setTriggerMode(TriggerMode mode) {
        if(!isDeviceConnected())
            return ErrorCode::ERROR_CONNECTION;

        if(mode < TriggerMode::TRIGGERMODE_AUTO || mode > TriggerMode::TRIGGERMODE_SINGLE)
            return ErrorCode::ERROR_PARAMETER;

        _settings.trigger.mode = mode;
        return ErrorCode::ERROR_NONE;
    }
}
