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
#include <cstring>
#include <fcntl.h>

#include "utils/containerStream.h"

#include "sineWaveDevice.h"
#include "utils/timestampDebug.h"
#include "utils/stdStringSplit.h"
#include "dsoSettings.h"


namespace DemoDevices {

SineWaveDevice::SineWaveDevice()
    : DeviceDummy(DSO::DSODeviceDescription()) {
    std::cout << "SineWaveDevice:: create deviceDummy" << std::endl;
}

SineWaveDevice::~SineWaveDevice() {
    disconnectDevice();
}

unsigned SineWaveDevice::getUniqueID() const {
    return 0;
}

bool SineWaveDevice::needFirmware() const {
    return false;
}

ErrorCode SineWaveDevice::uploadFirmware() {
    return ErrorCode::ERROR_NONE;
}

void SineWaveDevice::disconnectDevice() {
    if (!_thread.get()) return;
    _keep_thread_running = false;
    if (_thread->joinable()) _thread->join();
    _thread.reset();
}

bool SineWaveDevice::isDeviceConnected() const {
    return _thread.get();
}

void SineWaveDevice::connectDevice(){
    // Maximum possible samplerate for a single channel and dividers for record lengths
    std::cout << "SineWaveDevice::connectDevice()" << std::endl;
    _specification.channels         = 2;
    _specification.channels_special = 0;

    resetSettings();

    _specification.samplerate_single.base = 50e6;
    _specification.samplerate_single.max = 50e6;
    _specification.samplerate_single.maxDownsampler = 131072;
    _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(rollModeValue, 1000));
    _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(10240, 1));
    _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(32768, 1));
    _specification.samplerate_multi.base = 100e6;
    _specification.samplerate_multi.max = 100e6;
    _specification.samplerate_multi.maxDownsampler = 131072;
    _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(rollModeValue, 1000));
    _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(20480, 1));
    _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(65536, 1));
    _specification.sampleSize = 8;
    std::cout << "fill gainLevel" << std::endl;

/*
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 5,  2.5, 1.6,   255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 2,  5,   4,     255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10,   8,    255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10,  16,    255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10,  40,    255));
*/
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 2,  2.5,   1.6 ));   // 200mV/div
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 5,  5.0,    4   ));  // 500mV/div
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10.0,   8  ));   // 1V/div
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10.0,  16  ));   // 2V/div
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10.0,  40  ));   // 5V/div

    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_48MHZ,  48e6, DSO::HWRecordLengthID::RECORDLENGTH_1KB,2e-6,1));
//    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_30MHZ,  30e6, DSO::HWRecordLengthID::RECORDLENGTH_128KB,5e-6));
//    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_24MHZ,  24e6, DSO::HWRecordLengthID::RECORDLENGTH_128KB,5e-6));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_16MHZ,  16e6, DSO::HWRecordLengthID::RECORDLENGTH_1KB,5e-6,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_8MHZ,    8e6, DSO::HWRecordLengthID::RECORDLENGTH_1KB,10e-6,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_4MHZ,    4e6, DSO::HWRecordLengthID::RECORDLENGTH_1KB,20e-6,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_1MHZ,    1e6, DSO::HWRecordLengthID::RECORDLENGTH_1KB,50e-6,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_1MHZ,    1e6, DSO::HWRecordLengthID::RECORDLENGTH_1KB,100e-6,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_1MHZ,    1e6, DSO::HWRecordLengthID::RECORDLENGTH_2KB,200e-6,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_1MHZ,    1e6, DSO::HWRecordLengthID::RECORDLENGTH_5KB,500e-6,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_1MHZ,    1e6, DSO::HWRecordLengthID::RECORDLENGTH_10KB,1e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_1MHZ,    1e6, DSO::HWRecordLengthID::RECORDLENGTH_20KB,2e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_200KHZ,200e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,5e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_200KHZ,200e3, DSO::HWRecordLengthID::RECORDLENGTH_20KB,10e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_20KB,20e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,50e-3,5));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,100e-3,10));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,200e-3,20));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,500e-3,50));
//    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,1,100));

    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_1KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_2KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_5KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_10KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_20KB);

    _specification.availableCoupling.push_back(DSO::Coupling::COUPLING_NONE);
    this->_keep_thread_running = true;


    for (unsigned c=0; c < _specification.channels; ++c)
       getGainLevel(c).offset[c] = {0,255};

    std::cout << "SineWaveDevice::connectDevice() min sample rate: " << getMinSamplerate() << std::endl;

//    setSamplerate(getMinSamplerate());
    setSamplerate(DSO::HWSamplingRateID::SAMPLING_1MHZ);
    setRecordLengthByID(DSO::HWRecordLengthID::RECORDLENGTH_10KB);
    // _signals for initial _settings
//    notifySamplerateLimitsChanged();
    _recordLengthChanged(_settings.recordTypeID);
/*
    if(!isRollingMode())
        _recordTimeChanged((double) getCurrentRecordType().length_per_channel / _settings.samplerate.current);
    _samplerateChanged(_settings.samplerate.current);
*/
    _sampling = false;
    /*
     * to switch sampling nothing is needed on the dummy device
     */
    _samplingStarted =  [this]() {};
    _samplingStopped =  [this]() {};

    // The control loop is running until the device is disconnected

    setOffset(0, 0.5);
    setOffset(1, 0.5);

    std::cout << "running thread to generate data" << std::endl;
    _thread = std::unique_ptr<std::thread>(new std::thread(&SineWaveDevice::run,std::ref(*this)));

    _deviceConnected();


}

#include <random>
void SineWaveDevice::run() {

    FILE *fd;
    double x,xOffset;
    int xOffsetInt;
    std::cout << "SineWaveDevice::run()" << std::endl;
    std::vector<unsigned char> data;
    std::vector<unsigned char> ch1Data;           // data for 1 period
    std::vector<unsigned char> ch2Data;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.9,1.0);
    std::uniform_real_distribution<> horOffsetDis(0.0,1.0);
    int rawSine;

    if (!_keep_thread_running)
            std::cout << " data creation thread is not kept running" << std::endl;

    while (_keep_thread_running) {
        if (!this->_sampling) {
            std::cout << "SineWaveDevice::run() not sampling" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        else
            std::cout << "SineWaveDevice::run() sampling switched on" << std::endl;

        unsigned samples = 2*getExpectedRecordLength();
        double downSampling = _settings.samplerate.downsampler;
        std::cout << "SineWaveDemo: no of samples: " << samples << " downsampler: " << downSampling << std::endl;

        double hwgainCh0 = _settings.voltage[0].gainID;
        double gainCh0= _settings.voltage[0].gain;
        double multFactorCh0 = hwgainCh0/(gainCh0*DIVS_VOLTAGE);
        double samplingRate = getSamplerate();
        int recordSize = _settings.recordTypeID;
        std::cout << "sampling rate: " << samplingRate << " recordSize: "<< recordSize<< std::endl;

        std::cout << "on channel 0: hwgain: " << hwgainCh0 << " volts/div: " << gainCh0 << " mult factor: " << multFactorCh0<< std::endl;

        /*
         * now for channel 1
         */
        double hwgainCh1 = _settings.voltage[1].gainID;
        double gainCh1= _settings.voltage[1].gain;
        double multFactorCh1 = hwgainCh1/(gainCh1*DIVS_VOLTAGE);
        std::cout << "on channel 1: gain index: " << hwgainCh1 << " volts/div: " << gainCh1 << " mult factor: " << multFactorCh1<< std::endl;

        /*
         * create a 1 V, 1 kHz square wave
         */
        int upperLevel = 256.0/(gainCh1*multFactorCh1*DIVS_VOLTAGE)+128;          // 128 is the zero level
        std::cout << "upperLevel: " << upperLevel << std::endl;
        if (upperLevel > 255)
            upperLevel = 255; // saturation
        int lowerLevel = 128;
        int level;

        int switchPoint = (int)(samplingRate/(2000.0*downSampling));
        if (switchPoint < 2)
            switchPoint = 2;
        std::cout << "switch point: " << switchPoint<< std::endl;
 ;

/*
 * amplitude for sine wave
 */
        double sineAmplitude = 256/(gainCh0*multFactorCh0*DIVS_VOLTAGE);
        std::cout << "sineAmplitude: " << sineAmplitude << std::endl;
        const bool isFastRate = false;
        if (isFastRate) {
//        if (isFastRate()) {
            std::cout << "SineWaveDevice::run in fast mode" << std::endl;
            data.resize(samples);
            for (unsigned i=0;i<data.size();++i) {
                data[i] = sin(x)*dis(gen)*255;
                /* jump back and forth to test triggering */
                x += 1000.0*downSampling/samplingRate;    // should correspond to 1 kHz and does so if the sampling rate is 1 MHz
            }
        } else {
            std::cout << "SineWaveDevice::run in normal mode" << std::endl;
            data.resize(samples);
            /* generate a horizontal offset which will allow to test triggering */
            xOffset = horOffsetDis(gen);
            xOffsetInt = xOffset*samplingRate/(1000.0*downSampling);
            if (xOffsetInt < switchPoint)
                level = lowerLevel;
            else
                level = upperLevel;
//            std::cout << "x offset: " << xOffset << "x int: " << xOffsetInt << std::endl;

            x=xOffset;
            unsigned int j;
            for (unsigned int i=0;i<data.size()/2;i++) {
                rawSine = sin((2*M_PI*x))*(sineAmplitude)*dis(gen)+ 128;
                if (rawSine > 255)
                    rawSine =255;
                if (rawSine < 0)
                    rawSine = 0;
                data[2*i] = rawSine;
                //                data[i] = (sin(x))*dis(gen)*64;        // sin(x+shift_factor)*dis(gen)*128;
                j = i+xOffsetInt;
                if (!(j%switchPoint)) {
//                    std::cout << "switched with j= " << j << " i= " << i<< std::endl;
                    if (level == upperLevel)
                        level = lowerLevel;
                    else
                        level = upperLevel;
                }
                data[2*i+1]=level;  // rectangular wave at 1 kHz
                //                data[i+1] = int(x) % 255;             // triangle
                x += 1000.0*downSampling/samplingRate;
            }
/*
            fd=fopen("/tmp/sine.txt","w");
            fprintf(fd,"# x offset: %10.4f",xOffset);
            for (int i=0;i<4000;i+=2)
                fprintf(fd,"%d\n",data[i]);
            fclose(fd);

            fd=fopen("/tmp/square.txt","w");
            fprintf(fd,"# x offset: %d",xOffsetInt);
            for (int i=1;i<4000;i+=2)
                fprintf(fd,"%d\n",data[i]);
            fclose(fd);
*/
        }

        processSamples(data);

        std::cout << "calling dataAnalyzer:: data_from_device" << std::endl;
        _samplesAvailable(_samples);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    _statusMessage((int)ErrorCode::ERROR_NONE);
}

}
