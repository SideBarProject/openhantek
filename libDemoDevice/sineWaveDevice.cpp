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

#include "utils/containerStream.h"

#include "sineWaveDevice.h"
#include "utils/timestampDebug.h"
#include "utils/stdStringSplit.h"

#include <QDebug>

namespace DemoDevices {

SineWaveDevice::SineWaveDevice()
    : DeviceDummy(DSO::DSODeviceDescription()) {
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
    qDebug() << "SineWaveDevice::connectDevice()" << endl;
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

    _specification.gainLevel.push_back(DSO::dsoGainLevel(1,  1, 255));


    for (unsigned c=0; c < _specification.channels; ++c)
       getGainLevel(c).offset[c] = {0,255};

    setSamplerate(getMinSamplerate());

    // _signals for initial _settings
    notifySamplerateLimitsChanged();
    _recordLengthChanged(_settings.recordTypeID);
    if(!isRollingMode())
        _recordTimeChanged((double) getCurrentRecordType().length_per_channel / _settings.samplerate.current);
    _samplerateChanged(_settings.samplerate.current);

    _sampling = false;
    // The control loop is running until the device is disconnected
    _keep_thread_running = true;
    _thread = std::unique_ptr<std::thread>(new std::thread(&SineWaveDevice::run,std::ref(*this)));

    setOffset(0, 0.5);
    setOffset(1, 0.5);

    _deviceConnected();
}

#include <random>
void SineWaveDevice::run() {
    double x;

    std::vector<unsigned char> data;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.9,1.0);

    while (_keep_thread_running) {
        unsigned samples = getExpectedRecordLength();

        if (isFastRate()) {
            data.resize(samples);
            for (unsigned i=0;i<data.size();++i) {
                data[i] = sin(x)*dis(gen)*255;
                x += 0.01;
            }
        } else {
            data.resize(samples);
            for (unsigned i=0;i<data.size();i+=2) {
                data[i] = (sin(x)+1.0)*dis(gen)*(255/2);
                data[i+1] = int(x*10) % 255; // sin(x+shift_factor)*dis(gen)*128;
                x += 0.01;
            }
        }

        processSamples(data);

        _samplesAvailable(_samples);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    _statusMessage((int)ErrorCode::ERROR_NONE);
}

}
