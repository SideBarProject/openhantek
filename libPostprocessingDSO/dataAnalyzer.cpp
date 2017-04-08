////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  dataanalyzer.cpp
//
//  Copyright (C) 2010  Oliver Haag
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
#include <fftw3.h>
#include "dataAnalyzer.h"
#include "deviceBase.h"
#include "errorcodes.h"
#include "utils/timestampDebug.h"

namespace DSOAnalyser {

DataAnalyzer::DataAnalyzer(std::shared_ptr<DSO::DeviceBase> device, AnalyserSettings* analyserSettings)
    : _analyserSettings(analyserSettings), _device(device) {
        // lock analyse thread mutex
        _new_data_arrived_mutex.lock();

        // Connect to device
        using namespace std::placeholders;
        _device->_samplesAvailable = std::bind(&DataAnalyzer::data_from_device, this, _1);

        _analyserSettings->spectrumEnabled.resize(_device->getChannelCount());

        // Create thread
        _keep_thread_running = true;
        _thread = std::unique_ptr<std::thread>(new std::thread(&DataAnalyzer::analyseThread,std::ref(*this)));
    }

DataAnalyzer::~DataAnalyzer() {
    _analyzed = [](){};
    if (!_thread.get()) return;
    _keep_thread_running = false;
    _new_data_arrived_mutex.unlock();
    if (_thread->joinable()) _thread->join();
    _thread.reset();
    _device->_samplesAvailable = [](const std::vector<std::vector<double> >&){};
}

/// \brief Returns the analyzed data.
/// \param channel Channel, whose data should be returned.
/// \return Analyzed data as AnalyzedData struct.
AnalyzedData const *DataAnalyzer::data(unsigned channel) const {
    if(channel >= this->_analyzedData.size())
        return nullptr;

    return &this->_analyzedData[channel];
}

/// \brief Returns the sample count of the analyzed data.
/// \return The maximum sample count of the last analyzed data.
unsigned DataAnalyzer::sampleCount() const {
    return _maxSamples;
}

/// \brief Returns the mutex for the data.
/// \return Mutex for the analyzed data.
std::mutex& DataAnalyzer::mutex() {
    return _data_in_use_mutex;
}

void DataAnalyzer::copySamples(const std::vector<std::vector<double>>& incomingData, double samplerate, bool append) {
    size_t maxSamples = 0;

    // Adapt the number of channels for analyzed data
    this->_analyzedData.resize(incomingData.size());

    for(unsigned channel = 0; channel < incomingData.size(); ++channel) {
        AnalyzedData *const channelData = &this->_analyzedData[channel];

        if (incomingData[channel].empty()) {
            // Clear unused channels
            channelData->samples.voltage.sample.clear();
            channelData->samples.voltage.interval = 0;
            continue;
        }

        // Set sampling interval
        const double interval = 1.0 / samplerate;
        if(interval != channelData->samples.voltage.interval) {
            channelData->samples.voltage.interval = interval;
            if(append) // Clear roll buffer if the samplerate changed
                channelData->samples.voltage.sample.clear();
        }

        // Copy the buffer of the oscilloscope into the sample buffer
        if(append)
            channelData->samples.voltage.sample.insert(channelData->samples.voltage.sample.end(),
                                                       incomingData.at(channel).begin(),
                                                       incomingData.at(channel).end());
        else
            channelData->samples.voltage.sample = incomingData.at(channel);

        maxSamples = std::max(channelData->samples.voltage.sample.size(), maxSamples);
    }
    _maxSamples = maxSamples;
}

void DataAnalyzer::computeMathChannels()
{
    if (!_analyserSettings->mathChannelEnabled || _device->getChannelCount()<2)
        return;

    unsigned math_channel_id = _device->getChannelCount();
    _analyzedData.resize(math_channel_id+1);

    // Calculate values and write them into the sample buffer
    std::vector<double>::const_iterator ch1Iterator = _analyzedData[0].samples.voltage.sample.begin();
    std::vector<double>::const_iterator ch2Iterator = _analyzedData[1].samples.voltage.sample.begin();
    std::vector<double> &resultData = this->_analyzedData[math_channel_id].samples.voltage.sample;
    switch(_analyserSettings->mathmode) {
        case MathMode::ADD_CH1_CH2:
            for(unsigned i=0;i<_maxSamples;++i)
                resultData.push_back(*(ch1Iterator++) + *(ch2Iterator++));
            break;
        case MathMode::SUB_CH2_FROM_CH1:
            for(unsigned i=0;i<_maxSamples;++i)
                resultData.push_back(*(ch1Iterator++) - *(ch2Iterator++));
            break;
        case MathMode::SUB_CH1_FROM_CH2:
            for(unsigned i=0;i<_maxSamples;++i)
                resultData.push_back(*(ch2Iterator++) - *(ch1Iterator++));
            break;
    }
}

void DataAnalyzer::computeFreqSpectrumPeak(unsigned& lastRecordLength, WindowFunction& lastWindow, double *window) {
    for(unsigned channel = 0; channel < this->_analyzedData.size(); ++channel) {
        AnalyzedData *const channelData = &this->_analyzedData[channel];
        if(channelData->samples.spectrum.sample.empty()) {
            // Clear unused channels
            channelData->samples.spectrum.interval = 0;
            channelData->samples.spectrum.sample.clear();
            continue;
        }

        // Calculate new window
        unsigned sampleCount = channelData->samples.voltage.sample.size();
        bool sampleCountChanged = lastRecordLength != sampleCount;
        if(lastWindow != _analyserSettings->spectrumWindow || sampleCountChanged) {
            if(sampleCountChanged || !window) {
                lastRecordLength = sampleCount;

                if(window)
                    fftw_free(window);
                window = (double *) fftw_malloc(sizeof(double) * lastRecordLength);
            }

            unsigned windowEnd = lastRecordLength - 1;
            lastWindow = _analyserSettings->spectrumWindow;

            switch(_analyserSettings->spectrumWindow) {
                case WINDOW_HAMMING:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 0.54 - 0.46 * cos(2.0 * M_PI * windowPosition / windowEnd);
                    break;
                case WINDOW_HANN:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 0.5 * (1.0 - cos(2.0 * M_PI * windowPosition / windowEnd));
                    break;
                case WINDOW_COSINE:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = sin(M_PI * windowPosition / windowEnd);
                    break;
                case WINDOW_LANCZOS:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition) {
                        double sincParameter = (2.0 * windowPosition / windowEnd - 1.0) * M_PI;
                        if(sincParameter == 0)
                            *(window + windowPosition) = 1;
                        else
                            *(window + windowPosition) = sin(sincParameter) / sincParameter;
                    }
                    break;
                case WINDOW_BARTLETT:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 2.0 / windowEnd * (windowEnd / 2 - abs(windowPosition - windowEnd / 2));
                    break;
                case WINDOW_TRIANGULAR:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 2.0 / lastRecordLength * (lastRecordLength / 2 - abs(windowPosition - windowEnd / 2));
                    break;
                case WINDOW_GAUSS:
                    {
                        double sigma = 0.4;
                        for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                            *(window + windowPosition) = exp(-0.5 * pow(((windowPosition - windowEnd / 2) / (sigma * windowEnd / 2)), 2));
                    }
                    break;
                case WINDOW_BARTLETTHANN:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 0.62 - 0.48 * abs(windowPosition / windowEnd - 0.5) - 0.38 * cos(2.0 * M_PI * windowPosition / windowEnd);
                    break;
                case WINDOW_BLACKMAN:
                    {
                        double alpha = 0.16;
                        for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                            *(window + windowPosition) = (1 - alpha) / 2 - 0.5 * cos(2.0 * M_PI * windowPosition / windowEnd) + alpha / 2 * cos(4.0 * M_PI * windowPosition / windowEnd);
                    }
                    break;
                //case WINDOW_KAISER:
                    //TODO Spectrum WINDOW_KAISER
                    //double alpha = 3.0;
                    //for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        //*(window + windowPosition) = ;
                    //break;
                case WINDOW_NUTTALL:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 0.355768 - 0.487396 * cos(2 * M_PI * windowPosition / windowEnd) + 0.144232 * cos(4 * M_PI * windowPosition / windowEnd) - 0.012604 * cos(6 * M_PI * windowPosition / windowEnd);
                    break;
                case WINDOW_BLACKMANHARRIS:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 0.35875 - 0.48829 * cos(2 * M_PI * windowPosition / windowEnd) + 0.14128 * cos(4 * M_PI * windowPosition / windowEnd) - 0.01168 * cos(6 * M_PI * windowPosition / windowEnd);
                    break;
                case WINDOW_BLACKMANNUTTALL:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 0.3635819 - 0.4891775 * cos(2 * M_PI * windowPosition / windowEnd) + 0.1365995 * cos(4 * M_PI * windowPosition / windowEnd) - 0.0106411 * cos(6 * M_PI * windowPosition / windowEnd);
                    break;
                case WINDOW_FLATTOP:
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 1.0 - 1.93 * cos(2 * M_PI * windowPosition / windowEnd) + 1.29 * cos(4 * M_PI * windowPosition / windowEnd) - 0.388 * cos(6 * M_PI * windowPosition / windowEnd) + 0.032 * cos(8 * M_PI * windowPosition / windowEnd);
                    break;
                default: // WINDOW_RECTANGULAR
                    for(unsigned windowPosition = 0; windowPosition < lastRecordLength; ++windowPosition)
                        *(window + windowPosition) = 1.0;
            }
        }

        // Set sampling interval
        channelData->samples.spectrum.interval = 1.0 / channelData->samples.voltage.interval / sampleCount;

        // Number of real/complex samples
        unsigned dftLength = sampleCount / 2;

        // Reallocate memory for samples if the sample count has changed
        channelData->samples.spectrum.sample.resize(sampleCount);

        // Create sample buffer and apply window
        m_windowedValues.resize(sampleCount);
        m_correlation.resize(sampleCount);

        for(unsigned position = 0; position < sampleCount; ++position)
            m_windowedValues[position] = window[position] * channelData->samples.voltage.sample[position];

        // Do discrete real to half-complex transformation
        /// \todo Check if record length is multiple of 2
        /// \todo Reuse plan and use FFTW_MEASURE to get fastest algorithm
        fftw_plan fftPlan = fftw_plan_r2r_1d(sampleCount, &m_windowedValues[0],
                                             &channelData->samples.spectrum.sample.front(),
                                             FFTW_R2HC, FFTW_ESTIMATE);
        fftw_execute(fftPlan);
        fftw_destroy_plan(fftPlan);

        // Do an autocorrelation to get the frequency of the signal
        double *conjugateComplex = &m_windowedValues[0]; // Reuse the windowedValues buffer

        // Real values
        unsigned position;
        double correctionFactor = 1.0 / dftLength / dftLength;
        conjugateComplex[0] = (channelData->samples.spectrum.sample[0] * channelData->samples.spectrum.sample[0]) * correctionFactor;
        for(position = 1; position < dftLength; ++position)
            conjugateComplex[position] = (channelData->samples.spectrum.sample[position] * channelData->samples.spectrum.sample[position] + channelData->samples.spectrum.sample[sampleCount - position] * channelData->samples.spectrum.sample[sampleCount - position]) * correctionFactor;
        // Complex values, all zero for autocorrelation
        conjugateComplex[dftLength] = (channelData->samples.spectrum.sample[dftLength] * channelData->samples.spectrum.sample[dftLength]) * correctionFactor;
        for(++position; position < sampleCount; ++position)
            conjugateComplex[position] = 0;

        // Do half-complex to real inverse transformation
        fftPlan = fftw_plan_r2r_1d(sampleCount, conjugateComplex, &m_correlation[0], FFTW_HC2R, FFTW_ESTIMATE);
        fftw_execute(fftPlan);
        fftw_destroy_plan(fftPlan);

        // Calculate peak-to-peak voltage
        double minimalVoltage, maximalVoltage;
        minimalVoltage = maximalVoltage = channelData->samples.voltage.sample[0];

        for(unsigned position = 1; position < sampleCount; ++position) {
            if(channelData->samples.voltage.sample[position] < minimalVoltage)
                minimalVoltage = channelData->samples.voltage.sample[position];
            else if(channelData->samples.voltage.sample[position] > maximalVoltage)
                maximalVoltage = channelData->samples.voltage.sample[position];
        }

        channelData->amplitude = maximalVoltage - minimalVoltage;

        // Get the frequency from the correlation results
        double minimumCorrelation = m_correlation[0];
        double peakCorrelation = 0;
        unsigned peakPosition = 0;

        for(unsigned position = 1; position < sampleCount / 2; ++position) {
            if(m_correlation[position] > peakCorrelation && m_correlation[position] > minimumCorrelation * 2) {
                peakCorrelation = m_correlation[position];
                peakPosition = position;
            }
            else if(m_correlation[position] < minimumCorrelation)
                minimumCorrelation = m_correlation[position];
        }

        // Calculate the frequency in Hz
        if(peakPosition)
            channelData->frequency = 1.0 / (channelData->samples.voltage.interval * peakPosition);
        else
            channelData->frequency = 0;

        // Finally calculate the real spectrum if we want it
        if(_analyserSettings->spectrumEnabled[channel]) {
            // Convert values into dB (Relative to the reference level)
            double offset = 60 - _analyserSettings->spectrumReference - 20 * log10(dftLength);
            double offsetLimit = _analyserSettings->spectrumLimit - _analyserSettings->spectrumReference;
            for(double& spectrumIterator: channelData->samples.spectrum.sample) {
                double value = 20 * log10(fabs(channelData->samples.spectrum.sample[position])) + offset;

                // Check if this value has to be limited
                if(offsetLimit > value)
                    value = offsetLimit;

                spectrumIterator = value;
            }
        }
    }
}

AnalyserSettings* DataAnalyzer::getAnalyserSettings() const
{
    return _analyserSettings;
}

void DataAnalyzer::setAnalyserSettings(AnalyserSettings* analyserSettings)
{
    _analyserSettings = analyserSettings;
}

std::shared_ptr<DSO::DeviceBase> DataAnalyzer::getDevice() const
{
    return _device;
}

void DataAnalyzer::analyseThread() {
    unsigned lastRecordLength = 0; ///< The record length of the previously analyzed data
    WindowFunction lastWindow     = WINDOW_UNDEFINED; ///< The previously used dft window function
    double *window                = nullptr; ///< The array for the dft window factors

    while(_keep_thread_running) {
        _new_data_arrived_mutex.lock();
        computeMathChannels();
        computeFreqSpectrumPeak(lastRecordLength, lastWindow, window);
        _analyzed();

        //static unsigned long id = 0;
        //(void)id;
        //timestampDebug("Analyzed packet " << id++);
    }
}

/// \brief Starts the analyzing of new input data.
/// \param data The data arrays with the input data.
/// \param size The sizes of the data arrays.
/// \param samplerate The samplerate for all input data.
/// \param append The data will be appended to the previously analyzed data (Roll mode).
/// \param mutex The mutex for all input data.
void DataAnalyzer::data_from_device(const std::vector<std::vector<double>>& data) {
    // Lock the device thread, make a copy of the sample data, unlock the device thread.
    // Previous analysis still running, drop the new data
    if(!_data_in_use_mutex.try_lock()) {
        timestampDebug("Analyzer overload, dropping packets!");
        return;
    }
    copySamples(data, _device->getSamplerate(), _device->isRollingMode());
    _new_data_arrived_mutex.unlock(); ///< New data arrived, unlock analyse thread
}

}
