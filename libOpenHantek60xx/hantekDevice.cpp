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
#include <fcntl.h>

#include "utils/containerStream.h"

#include <libusb-1.0/libusb.h>

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
ErrorCode HantekDevice::resetDevice(bool enable) {
    unsigned char reset = (enable) ? 1 : 0;
    if (reset)
        std::cout << "Forcing Cypress chip into reset" << std::endl;
    else
        std::cout << "Releasing Cypress chip from reset" << std::endl;
    int error_code = _device->controlWrite(HT6022_FIRMWARE_REQUEST, &reset, 1, 0xe600, HT6022_FIRMWARE_INDEX);
    if (error_code < 0) {
        std::cout << "error when forcing Cypress chip into reset " << std::endl;
        _device->disconnect();
        return ErrorCode::ERROR_CONNECTION;
    }
    return ErrorCode::ERROR_NONE;
}

ErrorCode HantekDevice::uploadFirmware() {

    std::string line;
    std::string filename=_settings.firmwareFilename;
    std::cout << "HantekDevice::uploadFirmware: " << _settings.firmwareFilename << std::endl;
    unsigned char *dataBuffer;
    unsigned char checksum,checksumInFile;
    std::string noOfBytesString;
    std::string addressString;
    std::string dataByteString;
    std::stringstream ss;
    unsigned short noOfBytes,address;
    int i;

    if (!std::ifstream(filename)) {
        std::cout << "File " << filename << "does not exist" ;
        return ErrorCode::ERROR_BAD_FIRMWARE_FILENAME;
    }
    int error_code = _device->connect();
    if (error_code != LIBUSB_SUCCESS) {
        libusb_error_name(error_code);
        std::cerr << "Firmware upload: " << " " <<
        libusb_error_name((libusb_error)error_code) << " " <<
            libusb_strerror((libusb_error)error_code) << std::endl;
        return (ErrorCode)error_code;
    }

    resetDevice(true);
    std::ifstream infile(filename);
    std::cout << "uploading firmware from file " << filename << std::endl;

    while (getline(infile, line))
    {
        if (line.at(8)==END_OF_RECORD)
            break;
        noOfBytesString = line.substr(1,2);
        addressString = line.substr(3,4);
        std::cout << "no Of bytes: " << noOfBytesString << " address: " << addressString << std::endl;
        noOfBytes = stoi(noOfBytesString,nullptr,16);
        address = stoi(addressString,nullptr,16);
        std::cout << "no of Bytes (std::hex)" << std::hex << noOfBytes << " address: " << std::hex << address << std::endl;
 //       std::cout << line << std::endl;
        dataBuffer = new unsigned char[noOfBytes];
        for (i=0;i<noOfBytes;++i) {
            dataByteString = line.substr(9+i*2,2);
            dataBuffer[i] = stoi(dataByteString,nullptr,16);
            std::cout << std::hex << (int)dataBuffer[i] << " ";
        }
        checksumInFile = stoi(line.substr(9+i*2,2),nullptr,16);
        std::cout << std::endl;

        checksum = (address & 0xff) + (address >>8) + noOfBytes;
//       std::cout << "checksum with address and noOfBytes: " << std::hex << (int) checksum << " checksum in file: " << std::hex << (int) checksumInFile << endl;
        for (int i=0;i<noOfBytes;++i) {
            checksum += dataBuffer[i];
        }
        checksum =(~checksum) + 1;
        std::cout << "Checksum: " << std::hex << (int) checksum << " checksum in file: " << std::hex << (int) checksumInFile << std::endl;

        int error_code = _device->controlWrite(HT6022_FIRMWARE_REQUEST, dataBuffer, (unsigned int) noOfBytes, (int) address, HT6022_FIRMWARE_INDEX);
        if (error_code < 0) {
            std::cout << "load firmare, error after controlWrite " << std::endl;
            _device->disconnect();
            return ErrorCode::ERROR_CONNECTION;
        }
        delete[] dataBuffer;
    }
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
    std::cout << "HantekDevice::connectDevice()" << std::endl;
    if (_model.need_firmware) return;

    _statusMessage(_device->connect());
    if(!_device->isConnected())
            return;

    _specification.channels_special = 0;
    _specification.channels         = 2;

    resetPending();
    resetSettings();

    _specification.samplerate_single.base = 48e6;
    _specification.samplerate_single.max = 48e6;
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
/*
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1,   0.08, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1,   0.16, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1,   0.40, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1,   0.80, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1,   1.60, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 5,  2.5, 4.00, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 2,  5,   8.00, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 10, 1,  16.00, 255));
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 10, 1,  40.00, 255));
*/
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1.0,    0.08 ));  // 10  mV
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1.0,    0.16 ));  // 20  mV
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1.0,    0.40 ));  // 50  mV
    _specification.gainLevel.push_back(DSO::dsoGainLevel(10,  1.0,    0.80 ));  // 100 mV
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 5,  2.5,    1.60 ));  // 200 mV
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 2,  5.0,    4.00 ));  // 500 mV
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10.0,   8.00 ));  // 1V
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10.0,  16.00 ));  // 2V
    _specification.gainLevel.push_back(DSO::dsoGainLevel( 1,  10.0,  40.00 ));  // 5V

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
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_500KHZ,200e3, DSO::HWRecordLengthID::RECORDLENGTH_20KB,5e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_200KHZ,200e3, DSO::HWRecordLengthID::RECORDLENGTH_20KB,10e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_20KB,20e-3,1));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,50e-3,5));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,100e-3,10));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,200e-3,20));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,500e-3,50));
    _specification.availableSamplingRates.push_back(DSO::dsoAvailableSamplingRate(DSO::HWSamplingRateID::SAMPLING_100KHZ,100e3, DSO::HWRecordLengthID::RECORDLENGTH_10KB,1,100));

    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_1KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_2KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_5KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_10KB);
    _specification.availableRecordLengths.push_back(DSO::RECORDLENGTH_20KB);

    _specification.availableCoupling.push_back(DSO::Coupling::COUPLING_DC);

    std::cout << "setting data creation thread to keep running" << std::endl;
    this->_keep_thread_running = true;

    for (unsigned c=0; c < _specification.channels; ++c)
       getGainLevel(c).offset[c] = {0,255};

    std::cout << "set sample rate to " << getMinSamplerate() << std::endl;
    setSamplerate(getMinSamplerate());

    // _signals for initial _settings
    std::cout << "hantek::connectDevice() notify settings changed " << std::endl;
//    notifySamplerateLimitsChanged();
    _recordLengthChanged(_settings.recordTypeID);
    std::cout << "HantekDevice::connectDevice() _settings.samplerate.current: " << _settings.samplerate.current << std::endl;
//    if(!isRollingMode())
//        _recordTimeChanged((double) getCurrentRecordType().length_per_channel / _settings.samplerate.current);
//    _samplerateChanged(_settings.samplerate.current);

    _sampling = false;
    /*
     * here we switch sampling on the hardware on and off
     */
    _samplingStarted =  [this]() {

    };
    _samplingStopped =  [this]() {

    };

    // The control loop is running until the device is disconnected
    _keep_thread_running = true;
    _thread = std::unique_ptr<std::thread>(new std::thread(&HantekDevice::run,std::ref(*this)));

    _deviceConnected();

}

void HantekDevice::updatePretriggerPosition(double pretrigger_pos_in_s) {}

void HantekDevice::updateRecordLength(unsigned int index) {}

void HantekDevice::updateSamplerate(DSO::ControlSamplerateLimits *limits, unsigned int downsampler, bool fastRate) {}

ErrorCode HantekDevice::updateGain(unsigned channel, unsigned char hwGainCode)
{
    std::cout << "hantekDevice::updateGain on channel: " << channel <<" hw gain Code: " << (int) hwGainCode << std::endl;
    unsigned char sensitivityRequest;
    unsigned char newGain = hwGainCode;
    if (channel == 0)
        sensitivityRequest = HT6022_CH1_VR_REQUEST;
    if (channel == 1)
        sensitivityRequest = HT6022_CH2_VR_REQUEST;

    int error_code = _device->controlWrite(sensitivityRequest, &newGain , 1, HT6022_CH1_VR_VALUE , HT6022_FIRMWARE_INDEX);

    if (error_code < 0) {
        std::cout << "error " << error_code << " when setting gain on channel " <<channel << " to " << (int) newGain << std::endl;
        _device->disconnect();
        return ErrorCode::ERROR_CONNECTION;
    }
    return ErrorCode::ERROR_NONE;
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
    int retCode;
    FILE *fd_ch1,*fd_ch2;
    fd_ch1 = fopen("/tmp/traceDataCh1.txt","w");
    fd_ch2 = fopen("/tmp/traceDataCh2.txt","w");

    std::cout << "read " << (int)_dataSize*2 << "samples " << std::endl;
    *_data = HT6022_READ_CONTROL_DATA;
    int errorCode = _device->controlWrite(HT6022_READ_CONTROL_REQUEST,
                                          _data,
                                          HT6022_READ_CONTROL_SIZE,
                                          HT6022_READ_CONTROL_VALUE,
                                          HT6022_READ_CONTROL_INDEX);
    if(errorCode < 0)
        return errorCode;

    retCode = _device->bulkReadMulti(_data, (int)_dataSize*2);

    std::cout << "ht6022ReadData: " << retCode << "samples read\n" << std::endl;
    for (int i=0;i<retCode;i+=2) {
      fprintf(fd_ch1,"%d\n",_data[i]);
      fprintf(fd_ch2,"%d\n",_data[i+1]);
    }
    fclose(fd_ch1);
    fclose(fd_ch2);

    return retCode;
}

void HantekDevice::run() {

    FILE *fd_ch1,*fd_ch2;
    fd_ch1 = fopen("/tmp/traceDataCh1.txt","w");
    fd_ch2 = fopen("/tmp/traceDataCh2.txt","w");

    unsigned char *dataPtr = _data;
    std::cout << "Hantek run thread " << std::endl;
    std::vector<unsigned char> data;

    while (_keep_thread_running) {
/*
 * if we are not sampling then there is no reason to read out the scope
 */
        if (!this->_sampling) {
            std::cout << "HantekDevice::run() not sampling" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            continue;
        }
        else
            std::cout << "HantekDevice::run() sampling switched on" << std::endl;

        if (!sendPendingCommands(_device.get())) break;

        data.resize((int)_dataSize*2);

        // Compute sleep time
        int cycleTime;

        // Check the current oscilloscope state everytime 25% of the time the buffer should be refilled
//       if(isRollingMode())
//           cycleTime = _device->getPacketSize() / (!isFastRate() ? 1 : _specification.channels);
//        else
//            cycleTime = getCurrentRecordType().length_per_channel;

        cycleTime = cycleTime / _settings.samplerate.current * 250;

        // Not more often than every 10 ms though but at least once every second
        cycleTime = std::max(std::min(10, cycleTime), 1000);

        data[0] = HT6022_READ_CONTROL_DATA;
        std::cout << "reading data from scope" << std::endl;
        int errorCode = _device->controlWrite(HT6022_READ_CONTROL_REQUEST,
                                              &data[0],
                                              HT6022_READ_CONTROL_SIZE,
                                              HT6022_READ_CONTROL_VALUE,
                                              HT6022_READ_CONTROL_INDEX);
        if(errorCode < 0) {
            std::cout << "error reading hantek samples " << std::endl;
            break;
        }
        int retCode = _device->bulkReadMulti(&data[0], (int)_dataSize*2);

        std::cout << "writing data to file " << std::endl;
        for (int i=0;i<retCode;i+=2) {
          fprintf(fd_ch1,"%d\n",data[i]);
          fprintf(fd_ch2,"%d\n",data[i+1]);
        }

//        data.resize((samples);
        std::cout << "processing the data " << std::endl;
        processSamples(data);

        std::cout << "calling dataAnalyzer:: data_from_device" << std::endl;
        _samplesAvailable(_samples);

//        std::this_thread::sleep_for(std::chrono::milliseconds(cycleTime));
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    _device->disconnect();
    _statusMessage(LIBUSB_ERROR_NO_DEVICE);
}

}
