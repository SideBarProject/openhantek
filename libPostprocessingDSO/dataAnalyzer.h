////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Declares the DataAnalyzer class.
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

#ifndef DATAANALYZER_H
#define DATAANALYZER_H

#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <memory>
#include <atomic>

#include "dataAnalyzerSettings.h"

namespace DSO {
    class DeviceBase;
}

namespace DSOAnalyzer {

////////////////////////////////////////////////////////////////////////////////
/// \struct SampleValues                                          dataanalyzer.h
/// \brief Struct for a array of sample values.
struct SampleValues {
    std::vector<double> sample; ///< Vector holding the sampling data
    double interval = 0.0; ///< The interval between two sample values
};

////////////////////////////////////////////////////////////////////////////////
/// \struct SampleData                                            dataanalyzer.h
/// \brief Struct for the sample value arrays.
struct SampleData {
    SampleValues voltage; ///< The time-domain voltage levels (V)
    SampleValues spectrum; ///< The frequency-domain power levels (dB)
};

////////////////////////////////////////////////////////////////////////////////
/// \struct AnalyzedData                                          dataanalyzer.h
/// \brief Struct for the analyzed data.
struct AnalyzedData {
    SampleData samples; ///< Voltage and spectrum values
    double amplitude = 0.0; ///< The amplitude of the signal
    double frequency = 0.0; ///< The frequency of the signal
};

////////////////////////////////////////////////////////////////////////////////
///
/// \brief Analyzes the data from the dso.
/// Calculates the spectrum and various data about the signal and saves the
/// time-/frequencysteps between two values.
class DataAnalyzer {
    public:
        DataAnalyzer(std::shared_ptr<DSO::DeviceBase> device, AnalyzerSettings *AnalyzerSettings);
        ~DataAnalyzer();

        const AnalyzedData *data(unsigned int channel) const;

        const std::vector<AnalyzedData>& getAllData() const {return _analyzedData;}
        unsigned int sampleCount() const;

        /// Return a mutex that have to be locked while the Analyzed data
        /// vector (acquired via data()) is in use and unlocked after that.
        /// This class can not continue analysing incoming data until the
        /// mutex is unlocked.
        std::mutex& mutex();

        /// Signal: Data has been analyzed. Get the data via
        /// data(), get the sample count via sampleCount().
        /// You must unlock the Analyzed mutex (get it by mutex())
        /// after you have done your processing on data()/getAllData().
        std::function<void()> _analyzed = [](){};

        std::shared_ptr<DSO::DeviceBase> getDevice() const;
        AnalyzerSettings* getAnalyzerSettings() const;
        void setAnalyzerSettings(AnalyzerSettings* AnalyzerSettings);

private:

        /// Analyze incoming data from a device in a separate thread. Will make a copy of data for this purpose.
        /// This method is connected to the device in the constructor.
        void data_from_device(const std::vector<std::vector<double> >& data);

        /// A separate thread that runs forever and Analyzes incoming data from a device.
        /// Works with a copy of the device data. An anaylse iteration is started as soon
        /// as _mutex is unlocked in data_from_device. _AnalyzeIsRunning will be set true
        /// for an iteration.
        void analyzeThread();
        /// Analyzes the data from the dso (in a separate thread).
        void copySamples(const std::vector<std::vector<double>>& incomingData, double samplerate, bool append);
        /// Computes the math channels
        void computeMathChannels();
        /// Calculate frequencies, peak-to-peak voltages and spectrums (in a separate thread).
        void computeFreqSpectrumPeak(unsigned& lastRecordLength, WindowFunction& lastWindow, double *window);

        ///////// Input /////////

        /// The settings necessary to Analyze and compute data.
        AnalyzerSettings *_analyzerSettings;
        //double _incoming_samplerate = 0.0;   ///< The samplerate of the input data

        ///////// Output /////////

        /// The analyzed data for each channel
        std::vector<AnalyzedData> _analyzedData;
        /// The maximum record length of the analyzed data
        unsigned int _maxSamples = 0;

        /// A mutex that looks the analysing process to allow only one computation at a time
        std::mutex _new_data_arrived_mutex;
        std::mutex _data_in_use_mutex;
        std::vector<double> m_windowedValues, m_correlation;
        std::unique_ptr<std::thread> _thread;
        bool _keep_thread_running;
        std::shared_ptr<DSO::DeviceBase> _device;
};
}
#endif

