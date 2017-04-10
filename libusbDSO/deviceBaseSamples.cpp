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

#include <iostream>
#include <algorithm>

#include "deviceBaseSamples.h"
#include "utils/timestampDebug.h"

namespace DSO {

std::vector<int>& operator<<(std::vector<int>& v, int x);
std::vector<unsigned short int>& operator<<(std::vector<unsigned short int>& v, unsigned short int x);

void DeviceBaseSamples::startSampling() {
    std::cout << "start sampling" << std::endl;
    _sampling = true;
   _samplingStarted();
}

void DeviceBaseSamples::stopSampling() {
     std::cout << "stop sampling" << std::endl;
   _sampling = false;
   _samplingStopped();
}

bool DeviceBaseSamples::toogleSampling() {
    _sampling = !_sampling;
    if (_sampling)
        _samplingStarted();
    else
        _samplingStopped();

    return _sampling;
}

double DeviceBaseSamples::getMinSamplerate() {
    std::cout << "DeviceBaseSamples::getMinSamplerate: samplerate_single.base " <<  _specification.samplerate_single.base << std::endl;
    std::cout << "DeviceBaseSamples::getMinSamplerate: single.maxDownsampler " <<  _specification.samplerate_single.maxDownsampler << std::endl;


    return (double) _specification.samplerate_single.base / _specification.samplerate_single.maxDownsampler;
}

double DeviceBaseSamples::getMaxSamplerate() {
    std::cout << "DeviceBaseSamples::getMaxSamplerate" << std::endl;
    ControlSamplerateLimits *limits = (_settings.usedChannels <= 1) ? &_specification.samplerate_multi : &_specification.samplerate_single;
    return limits->max;
}

double DeviceBaseSamples::getSamplerate()
{
    std::cout << "DeviceBaseSamples::getSamplerate: " << _settings.samplerate.current << std::endl;
    return _settings.samplerate.current;
}
/*
void DeviceBaseSamples::notifySamplerateLimitsChanged() {
    std::cout << "DeviceBaseSamples::notifySamplerateLimitsChanged() " << std::endl;
    recomputeSamplerate(_settings.samplerate.target_samplerate,
                        _specification.samplerate_single.max, false);

    unsigned divider = getCurrentRecordType().divider;
    // Works only if the minimum samplerate for normal mode is lower than for fast rate mode, which is the case for all models
    _samplerateLimitsChanged(getMinSamplerate() / divider, getMaxSamplerate() / divider);
}
*/
void DeviceBaseSamples::setSamplerate(double samplerate) {
/*
    std::cout << "DeviceBaseSamples::setSamplerate to " << samplerate << std::endl;
    if(samplerate == 0.0)
        throw std::runtime_error("setSamplerate with 0 not allowed");

    recomputeSamplerate(samplerate, _specification.samplerate_single.max, false);
*/
    _settings.samplerate.current = samplerate;
}
/*
void DeviceBaseSamples::setSamplerateByRecordTime(double duration_in_s) {
    if(duration_in_s <= 0.0)
        throw std::runtime_error("setRecordTime with 0 not allowed");

    // Calculate the maximum samplerate that would still provide the requested duration
    double maxSamplerate = getCurrentRecordType().length_per_channel / duration_in_s;

    recomputeSamplerate(maxSamplerate, _specification.samplerate_multi.base, true);
}
*/
void DeviceBaseSamples::setPreTriggerPosition(double pretrigger_pos_in_s)
{
    _settings.trigger.pretrigger_pos_in_s = pretrigger_pos_in_s;
    updatePretriggerPosition(pretrigger_pos_in_s);
}

/*
void DeviceBaseSamples::recomputeSamplerate(double samplerate, double baseSamplerate, bool maximum)
{
    // Update target samplerate
    std::cout << "DeviceBaseSamples::recomputeSamplerate with samplerate=" << samplerate << std::endl;

    _settings.samplerate.target_samplerate = samplerate;

    // Update fastRate
    bool fastRate = (_settings.usedChannels <= 1) &&
                    (samplerate >= baseSamplerate / getCurrentRecordType().divider);
    bool fastRateChanged = fastRate != isFastRate();
    if(fastRateChanged) {
        _settings.samplerate.limits = fastRate ? &_specification.samplerate_single : &_specification.samplerate_multi;
    }

    // What is the nearest, at least as high samplerate the scope can provide?
    unsigned int downsampler;
    double bestSamplerate;
    std::cout << "before tie" << std::endl;
    std::tie(bestSamplerate, downsampler) = computeBestSamplerate(samplerate, _settings.samplerate.limits, maximum);

    std::cout << "DeviceBaseSamples::updateSamplerate downsampler:" << downsampler << " fastrate: " << fastRate<< std::endl;
    updateSamplerate(_settings.samplerate.limits, downsampler, fastRate);

    _settings.samplerate.downsampler = downsampler;
    if (downsampler)
        std::cout << "DeviceBaseSamples::recomputeSamplerate downsampler" << std::endl;
    else
        std::cout << "DeviceBaseSamples::recomputeSamplerate no downsampler" << std::endl;
    std::cout << "DeviceBaseSamples::recomputeSamplerate limits->base: " << _settings.samplerate.limits->base << std::endl;
    std::cout << "DeviceBaseSamples::recomputeSamplerate limits->max: " << _settings.samplerate.limits->max << std::endl;
    std::cout << "DeviceBaseSamples::recomputeSamplerate divider: " << getCurrentRecordType().divider << std::endl;

    if(downsampler)
        _settings.samplerate.current = _settings.samplerate.limits->base / getCurrentRecordType().divider / downsampler;
    else
        _settings.samplerate.current = _settings.samplerate.limits->max / getCurrentRecordType().divider;

    // Update dependencies
    updatePretriggerPosition(_settings.trigger.pretrigger_pos_in_s);

    // Emit signals for changed settings
    if(fastRateChanged) {
        _recordLengthChanged(_settings.recordTypeID);
    }

    // Check for Roll mode
    if(!isRollingMode())
         _recordTimeChanged((double) getCurrentRecordType().length_per_channel / _settings.samplerate.current);

    _samplerateChanged(_settings.samplerate.current);
}

std::pair<double, unsigned int> DeviceBaseSamples::computeBestSamplerate(double samplerate, const ControlSamplerateLimits* limits, bool maximum) const
{

    // Abort if the input value is invalid
    std::cout << "DeviceBaseSamples::computeBestSamplerate: samplerate: " << samplerate << std::endl;
    if(samplerate == 0.0) {
        std::cout << "DeviceBaseSamples::computeBestSamplerate: samplerate is 0 and would throw exception" << std::endl;
        throw std::runtime_error("computeBestSamplerate with 0 not allowed");
    }
    double bestSamplerate = 0.0;

    // Get downsampling factor that would provide the requested rate

    std::cout << "DeviceBaseSamples::computeBestSamplerate: currentRecordType().divider: " << getCurrentRecordType().divider << std::endl;
    double bestDownsampler = (double) limits->base / getCurrentRecordType().divider / samplerate;

    // Base samplerate sufficient, or is the maximum better?
    if(bestDownsampler < 1.0 && (samplerate <= limits->max / getCurrentRecordType().divider || !maximum)) {
        bestDownsampler = 0.0;
        bestSamplerate = limits->max / getCurrentRecordType().divider;
        return std::pair<double, unsigned int>(bestSamplerate, bestDownsampler);
    }

    bestDownsampler = getDownsamplerRate(bestDownsampler, maximum);

    // Limit maximum downsampler value to avoid overflows in the sent commands
    if(bestDownsampler > limits->maxDownsampler)
        bestDownsampler = limits->maxDownsampler;

    bestSamplerate = limits->base / bestDownsampler / getCurrentRecordType().divider;
    return std::pair<double, unsigned int>(bestSamplerate, bestDownsampler);
}

double DeviceBaseSamples::getDownsamplerRate(double bestDownsampler, bool maximum) const
{
    if(maximum)
        return ceil(bestDownsampler); // Round up to next integer value
    else
        return floor(bestDownsampler); // Round down to next integer value
}
*/

unsigned int DeviceBaseSamples::getExpectedRecordLength() {
    return((unsigned int)_settings.samplerate.recordLengthID);

}

void DeviceBaseSamples::setRecordLengthByID(HWRecordLengthID recordTypeID) {
    _settings.samplerate.recordLengthID= recordTypeID;
}

/*
void DeviceBaseSamples::setRecordLengthByID(unsigned int recordTypeID) {
    updateRecordLength(recordTypeID);

    // Check if the divider has changed and adapt samplerate limits accordingly
    bool bDividerChanged = recordTypeID != _settings.recordTypeID;
    _settings.recordTypeID = recordTypeID;
    std::cout << "DeviceBaseSamples::setRecordLengthByID recordTypeID: " << recordTypeID << std::endl;

    if(bDividerChanged) {
        this->notifySamplerateLimitsChanged();

        // Samplerate dividers changed, recalculate it
        setSamplerate(_settings.samplerate.target_samplerate);
    }

    updatePretriggerPosition(_settings.trigger.pretrigger_pos_in_s);

    _recordLengthChanged(_settings.recordTypeID);
}
*/
void DeviceBaseSamples::processSamples(std::vector<unsigned char>& data) {
    unsigned sampleCountAllChannels;

    const bool samplesize_greater_byte = _specification.sampleSize > 8;

    // How much data did we really receive?
    if(samplesize_greater_byte)
        sampleCountAllChannels = data.size() / 2; // For 9bit-16bit Analog digital converters
    else
        sampleCountAllChannels = data.size();
    std::cout << "DeviceBaseSamples::processSamples sampleCountAllChannels: " << sampleCountAllChannels << std::endl;
    // Fast rate mode, one channel is using all buffers
    // Normal mode, channels are using their separate buffers
//    const bool fastRate = isFastRate();
    const bool fastRate=false;
    const unsigned sampleCount = fastRate ? sampleCountAllChannels : (sampleCountAllChannels / _specification.channels);
    const unsigned buffer_inc  = fastRate ? 1 : _specification.channels;
    std::cout << "DeviceBaseSamples::processSamples sampleCounts: " << sampleCount << std::endl;
    // Convert data from the oscilloscope and write it into the sample buffer
    // Additional most significant bits after the normal data
    const unsigned int extraBitsSize = _specification.sampleSize - 8; // Number of extra bits
    const unsigned short int extraBitsMask = (0x00ff << extraBitsSize) & 0xff00; // Mask for extra bits extraction

    // Convert channel data
    for(unsigned channel = 0; channel < _specification.channels; ++channel) {
        if(!_settings.voltage[channel].used) {
            // Clear unused channels
            _samples[channel].resize(0);
            continue;
        }

        // Resize sample vector
        _samples[channel].resize(sampleCount);

        const unsigned gainID  = _settings.voltage[channel].gainID;
        const double gain_limit = _specification.gainLevel[gainID].voltage;
        const double gain       = _specification.gainLevel[gainID].gainSteps;
        const double offsetReal = _settings.voltage[channel].offsetReal;
        double extra_value      = 0; // For sample sizes of > 8 bit, the computed extra value is stored here.
        unsigned bufferPosition = _settings.trigger.point * 2;

        // Fastrate uses the entire buffer, no offset in the data buffer for different channels.
        // Non fastrate: The channels data are interleaved. If we have 2 channels for example,
        // all even buffer positions are chan0, all uneven positions belong to chan1.
        const int chanOffset = fastRate ? 0 : _specification.channels - 1 - channel;
        std::cout << "DeviceBaseSamples::processSamples channel offset: " << chanOffset << std::endl;
        std::cout << "buffer position: " << bufferPosition <<" buffer increment " << buffer_inc << std::endl;

        for(unsigned sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex, bufferPosition += buffer_inc) {
            bufferPosition %= 2*sampleCount;

            if (samplesize_greater_byte) {
                if (fastRate) {
                    std::cout << "DeviceBaseSamples::processSamples fastRate" << std::endl;
                    // Track the position of the extra bits in the additional byte
                    unsigned int extraBitsPosition = bufferPosition % _specification.channels;
                    unsigned extraBitsIndex = 8 - (_specification.channels - 1 - extraBitsPosition) * extraBitsSize;
                    extra_value = (((unsigned short int) data[sampleCountAllChannels + bufferPosition - extraBitsPosition] << extraBitsIndex) & extraBitsMask);
                } else {
                    std::cout << "DeviceBaseSamples::processSamples not fastRate" << std::endl;
                    unsigned extraBitsIndex = 8 - channel * extraBitsSize; // Bit position offset for extra bits extraction
                    extra_value = (((unsigned short int) data[sampleCountAllChannels + bufferPosition] << extraBitsIndex) & extraBitsMask);
                }
            }

            const double value = data[bufferPosition + chanOffset] + extra_value;
//            _samples[channel][sampleIndex] = (value / gain_limit - offsetReal) * gain;
            _samples[channel][sampleIndex] = value;
        }
    }


    static unsigned id = 0;
    (void)id;
    timestampDebug("Received packet " << id++);
}

}
