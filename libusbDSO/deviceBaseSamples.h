////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//
/// \copyright (c) 2008, 2009 Oleg Khudyakov <prcoder@potrebitel.ru>
/// \copyright (c) 2010 - 2012 Oliver Haag <oliver.haag@gmail.com>
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

#pragma once

#include <functional>
#include <vector>
#include <climits>

#include "dsoSettings.h"
#include "dsoSpecification.h"
#include "errorcodes.h"
#include "deviceDescriptionEntry.h"
#include "deviceBaseSpecifications.h"

namespace DSO {
#define rollModeValue UINT_MAX

//////////////////////////////////////////////////////////////////////////////
/// \brief Part of the base class for an DSO device implementation. All sample
///        related parts are implemented here.
class DeviceBaseSamples : public DeviceBaseSpecifications {
public:
    DeviceBaseSamples(const DSODeviceDescription& model) : DeviceBaseSpecifications(model) {}

    /// \brief Get minimum samplerate for this oscilloscope.
    /// \return The minimum samplerate for the current configuration in S/s.
    double getMinSamplerate();

    /// \brief Get maximum samplerate for this oscilloscope.
    /// \return The maximum samplerate for the current configuration in S/s.
    double getMaxSamplerate();

    /// \brief Get the current samplerate.
    double getSamplerate();

    /// \brief Sets the samplerate of the oscilloscope.
    /// \param samplerate The samplerate that should be met (S/s). Have to be greater than 0.
    /// To restore the samplerate to the current one, call with _settings.samplerate.target.samplerate.
    void setSamplerate(double samplerate);

    /// \brief Sets the time duration of one aquisition by adapting the samplerate.
    /// \param duration The record time duration (s) that should be met. Have to be greater than 0.
    void setSamplerateByRecordTime(double duration_in_s);

    /// \brief Return the number of samples accumulated for all enabled channels that are expected.
    unsigned int getExpectedRecordLength();

    /// \brief Sets the size of the oscilloscopes sample buffer.
    /// setPretriggerPosition() is called.
    /// \param recordTypeID The record length index that should be set.
    void setRecordLengthByID(unsigned int recordTypeID);

    /// \brief Set the trigger position.
    /// \param position The new trigger position (in s).
    virtual void setPreTriggerPosition(double pretrigger_pos_in_s);

    /// \brief Start sampling process.
    void startSampling();

    /// \brief Stop sampling process.
    void stopSampling();

    /// \brief Stop/Start sampling process.
    bool toogleSampling();

    inline bool isRollingMode() { return getCurrentRecordType().length_per_channel == rollModeValue; }
    inline bool isFastRate() { return _settings.samplerate.limits == &_specification.samplerate_multi;}
protected:
    /// \brief Sets the size of the sample buffer without updating dependencies.
    /// \param index The record length index that should be set.
    virtual void updateRecordLength(unsigned int index) = 0;

    /// \brief Sets the samplerate based on the parameters calculated by Control::computeBestSamplerate.
    /// \param limits The downsampling factor.
    /// \param downsampler The downsampling factor.
    /// \param fastRate true, if one channel uses all buffers.
    virtual void updateSamplerate(ControlSamplerateLimits *limits, unsigned int downsampler, bool fastRate) = 0;

    virtual void updatePretriggerPosition(double position) = 0;
public:
    /**
     * This section contains callback methods. Register your function or class method to get notified
     * of events.
     */

    /// The oscilloscope started sampling/waiting for trigger
    std::function<void(void)> _samplingStarted = [](){};
    /// The oscilloscope stopped sampling/waiting for trigger
    std::function<void(void)> _samplingStopped = [](){};

    /// New sample data is available as channel[data] vectors, the samplerate, rollMode flag
    /// and mutex to block the communication thread if necessary to parse the incoming data.
    std::function<void(const std::vector<std::vector<double>>&)> _samplesAvailable
        = [](const std::vector<std::vector<double>>&){};

    /// The available record lengths, empty list for continuous
    /// and the ID for the current record length.
    std::function<void(unsigned)> _recordLengthChanged = [](unsigned){};

    /// The minimum or maximum samplerate has changed
    std::function<void(double,double)> _samplerateLimitsChanged = [](double, double){};

    /// The record time duration has changed
    std::function<void(double)> _recordTimeChanged = [](double){};
    /// The samplerate has changed
    std::function<void(double)> _samplerateChanged = [](double){};

protected:
    /// \brief Converts raw input bytes to samples for each channel.
    /// The incoming data is assumed to be in the following format:
    /// a) Fastrate (the entire vector contains data of one channel only)
    ///    1) Samplesize==8bit: Each entry is one sample
    ///    2) Samplesize >8bit:
    ///       For the first half of the vector: Each entry is one sample.
    ///       For the second half of the vector: entry{x+half_size} contains additional bits for sample{x}
    /// b) Interleaved mode: Channel data is alternating in the vector. Example:
    ///    entry{0} = sample, chan0
    ///    entry{1} = sample, chan1
    ///    entry{2} = sample, chan0
    ///    entry{3} = sample, chan1
    ///    if Samplesize >8bit:
    ///       Additional bits are found in the second half of the vector like in a2.
    /// The result is saved in {@see DeviceBaseSamples::_samples}.
    /// You need to override or not use this method if your DSO works in a different way.
    void processSamples(std::vector<unsigned char>& data);

    /// \brief Notifies about the minimum and maximum supported samplerate.
    void notifySamplerateLimitsChanged();

    /// Recomputes samplerrate. Called by setSamplerate...
    void recomputeSamplerate(double samplerate, double baseSamplerate, bool maximum);

    /// \brief Calculate the nearest samplerate supported by the oscilloscope.
    /// This method is used by recomputeSamplerate(...).
    /// \param samplerate The target samplerate, that should be met as good as possible. Have to be greater than 0.
    /// \param limits The samplerrate limits.
    /// \param maximum The target samplerate is the maximum allowed when true, the minimum otherwise.
    /// \return pair(The nearest samplerate supported; Selected downsampling factor).
    virtual std::pair<double, unsigned int> computeBestSamplerate(double samplerate,
                                                                  const DSO::ControlSamplerateLimits* limits,
                                                                  bool maximum) const;

    virtual double getDownsamplerRate(double bestDownsampler, bool maximum) const;
protected:
    typedef std::vector<double> data_one_channel;
    std::vector<data_one_channel> _samples;    ///< Sample data vectors sent to the data analyzer
    bool _sampling;      ///< true, if the oscilloscope is taking samples
};

}
