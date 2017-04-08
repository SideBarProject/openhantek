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

#include "utils/containerStream.h"

#include <libusb-1.0/libusb.h>

#include "HT6022BEfw.h"
#include "HT6022BLfw.h"

#include "hantekDevice.h"
#include "utils/timestampDebug.h"
#include "utils/stdStringSplit.h"

namespace Hantek60xx {

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
    return _model.need_firmware;
}

ErrorCode HantekDevice::uploadFirmware() {
    int error_code = _device->connect();
    if (error_code != LIBUSB_SUCCESS) {
        libusb_error_name(error_code);
        std::cerr << "Firmware upload: " << " " <<
        libusb_error_name((libusb_error)error_code) << " " <<
            libusb_strerror((libusb_error)error_code) << std::endl;
        return (ErrorCode)error_code;
    }

    int fwsize = 0;
    unsigned char* firmware = nullptr;
    switch (_model.productID) {
        case 0X6022:
            fwsize = HT6022_FIRMWARE_SIZE;
            firmware = HT6022_Firmware;
            break;
        case 0X602A:
            fwsize = HT6022BL_FIRMWARE_SIZE;
            firmware = HT6022BL_Firmware;
            break;
        default:
            return ErrorCode::ERROR_PARAMETER;
    }

    unsigned int Size, Value;
    while (fwsize) {
        Size  = *firmware + ((*(firmware + 1))<<0x08);
        firmware = firmware + 2;
        Value = *firmware + ((*(firmware + 1))<<0x08);
        firmware = firmware + 2;
        int error_code = _device->controlWrite(HT6022_FIRMWARE_REQUEST,firmware,Size,Value,HT6022_FIRMWARE_INDEX);
        if (error_code < 0) {
            _device->disconnect();
            return ErrorCode::ERROR_CONNECTION;
        }
        firmware = firmware + Size;
        fwsize--;
    }

    _uploadProgress(100);
    _device->disconnect();

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
    _keep_thread_running = false;
}

bool HantekDevice::isDeviceConnected() const {
    return _device->isConnected();
}

void HantekDevice::connectDevice(){
    if (_model.need_firmware) return;

    if (_model.need_firmware) return;

    _statusMessage(_device->connect());
    if(!_device->isConnected())
            return;

    _specification.channels_special = 0;
    _specification.channels         = 2;

    resetPending();
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
    _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  0.08, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(1,  0.16, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(2,  0.40, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  0.80, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(1,  1.60, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(2,  4.00, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  8.00, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(1, 16.00, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(2, 40.00, 255));

    // _signals for initial _settings
    notifySamplerateLimitsChanged();
    _recordLengthChanged(_settings.recordTypeID);
    if(!isRollingMode())
        _recordTimeChanged((double) getCurrentRecordType().length_per_channel / _settings.samplerate.current);
    _samplerateChanged(_settings.samplerate.current);

    _sampling = false;
    // The control loop is running until the device is disconnected
    _keep_thread_running = true;
    _thread = std::unique_ptr<std::thread>(new std::thread(&HantekDevice::run,std::ref(*this)));

}

void HantekDevice::updatePretriggerPosition(double pretrigger_pos_in_s) {}

void HantekDevice::updateRecordLength(unsigned int index) {}

void HantekDevice::updateSamplerate(DSO::ControlSamplerateLimits *limits, unsigned int downsampler, bool fastRate) {}

void HantekDevice::updateGain(unsigned channel, unsigned char gainIndex, unsigned gainId)
{

}

void HantekDevice::updateOffset(unsigned int channel, unsigned short offsetValue)
{

}

ErrorCode HantekDevice::updateTriggerSource(bool special, unsigned int channel)
{
return ErrorCode::ERROR_NONE;
}

ErrorCode HantekDevice::updateTriggerLevel(unsigned int channel, double level)
{
return ErrorCode::ERROR_NONE;
}

ErrorCode HantekDevice::updateTriggerSlope(DSO::Slope slope)
{
return ErrorCode::ERROR_NONE;
}

int HantekDevice::readSamples() {
    *_data = HT6022_READ_CONTROL_DATA;
    int errorCode = _device->controlWrite(HT6022_READ_CONTROL_REQUEST,
                                          _data,
                                          HT6022_READ_CONTROL_SIZE,
                                          HT6022_READ_CONTROL_VALUE,
                                          HT6022_READ_CONTROL_INDEX);
    if(errorCode < 0)
        return errorCode;

    return _device->bulkReadMulti(_data, (int)_dataSize*2);
}

void HantekDevice::run() {
    while (_keep_thread_running) {
        if (!sendPendingCommands(_device.get())) break;

        // Compute sleep time
        int cycleTime;

        // Check the current oscilloscope state everytime 25% of the time the buffer should be refilled
        if(isRollingMode())
            cycleTime = _device->getPacketSize() / (!isFastRate() ? 1 : _specification.channels);
        else
            cycleTime = getCurrentRecordType().length_per_channel;

        cycleTime = cycleTime / _settings.samplerate.current * 250;

        // Not more often than every 10 ms though but at least once every second
        cycleTime = std::max(std::min(10, cycleTime), 1000);

        if (readSamples() < 0)
            break;

        //processSamples(_data, _dataSize);

        std::this_thread::sleep_for(std::chrono::milliseconds(cycleTime));
    }

    _device->disconnect();
    _statusMessage(LIBUSB_ERROR_NO_DEVICE);
}

}
