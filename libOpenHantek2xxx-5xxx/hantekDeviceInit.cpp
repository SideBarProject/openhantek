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

#include <libusb-1.0/libusb.h>

#include "protocol.h"
#include "hantekDevice.h"
#include "protocol.h"
#include "utils/timestampDebug.h"
#include "utils/stdStringSplit.h"

namespace Hantek2xxx_5xxx {

void HantekDevice::connectDevice() {
    if (_model.need_firmware) return;

    _statusMessage(_device->connect());
    if(!_device->isConnected())
        return;

    resetPending();

    // Determine the _command version we need for this model
    bool unsupported = false;
    switch(_model.productID) {
        case 0x2150:
            unsupported = true;

        case 0x2090:
            // Instantiate additional _commands for the DSO-2090. Initialize those as pending
            addPending(get<BulkSetTriggerAndSamplerate>());
            break;

        case 0x2250:
            // Instantiate additional _commands for the DSO-2250. Initialize those as pending
            addPending(get<BulkSetChannels2250>());
            addPending(get<BulkSetTrigger2250>());
            addPending(get<BulkSetRecordLength2250>());
            addPending(get<BulkSetSamplerate2250>());
            addPending(get<BulkSetBuffer2250>());
            break;

        case 0x520A:
            unsupported = true;

        case 0x5200:
            // Instantiate additional _commands for the DSO-5200. Initialize those as pending
            addPending(get<BulkSetSamplerate5200>());
            addPending(get<BulkSetBuffer5200>());
            addPending(get<BulkSetTrigger5200>());
            break;

        default:
            _device->disconnect();
            std::cerr <<"Model not supported by this driver!" << std::endl;
            _statusMessage(int(ErrorCode::ERROR_UNSUPPORTED));
            return;
    }

    if(unsupported)
        std::cerr <<"Warning: This Hantek DSO model isn't supported officially, so it may not be working as expected. Reports about your experiences are very welcome though (Please open a feature request in the tracker at http://www.github.com/openhantek/openhantek). If it's working perfectly this warning can be removed, if not it should be possible to get it working with your help soon." << std::endl;

    // Maximum possible samplerate for a single channel and dividers for record lengths
    _specification.channels         = 2;
    _specification.channels_special = 2;
    _specification.features = DSO::hasHardwareTrigger;

    resetSettings();

    _specification.specialTriggerSources = {"EXT", "EXT/10"};

    switch(_model.productID) {
        case 0x5200:
        case 0x520A:
            _specification.samplerate_single.base = 100e6;
            _specification.samplerate_single.max = 125e6;
            _specification.samplerate_single.maxDownsampler = 131072;
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(rollModeValue, 1000));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(10240, 1));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(14336, 1));
            _specification.samplerate_multi.base = 200e6;
            _specification.samplerate_multi.max = 250e6;
            _specification.samplerate_multi.maxDownsampler = 131072;
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(rollModeValue, 1000));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(20480, 1));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(28672, 1));
            _specification.sampleSize = 10;

            _specification.gainLevel.push_back(DSO::dsoGainLevel(1,  0.16, 368));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  0.40, 454));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  0.80, 908));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(1,  1.60, 368));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  4.00, 454));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  8.00, 908));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(1, 16.00, 368));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0, 40.00, 454));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0, 80.00, 908));

            break;

        case 0x2250:
            _specification.samplerate_single.base = 100e6;
            _specification.samplerate_single.max = 100e6;
            _specification.samplerate_single.maxDownsampler = 65536;
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(rollModeValue, 1000));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(10240, 1));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(524288, 1));
            _specification.samplerate_multi.base = 200e6;
            _specification.samplerate_multi.max = 250e6;
            _specification.samplerate_multi.maxDownsampler = 65536;
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(rollModeValue, 1000));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(20480, 1));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(1048576, 1));
            _specification.sampleSize = 8;

            _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  0.08, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(2,  0.16, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(3,  0.40, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  0.80, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(2,  1.60, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(3,  4.00, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(0,  8.00, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(2, 16.00, 255));
            _specification.gainLevel.push_back(DSO::dsoGainLevel(3, 40.00, 255));

            break;

        case 0x2150:
            _specification.samplerate_single.base = 50e6;
            _specification.samplerate_single.max = 75e6;
            _specification.samplerate_single.maxDownsampler = 131072;
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(rollModeValue, 1000));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(10240, 1));
            _specification.samplerate_single.recordTypes.push_back(DSO::dsoRecord(32768, 1));
            _specification.samplerate_multi.base = 100e6;
            _specification.samplerate_multi.max = 150e6;
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
            break;

        default:
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
            break;
    }

    // Get channel level data
    // channelLevels[channel][GainId][::LevelOffset]
    unsigned short int offsetLimit[_specification.channels][_specification.gainLevel.size()][2];
    int errorCode = _device->controlRead(CONTROL_VALUE, (unsigned char *) &(offsetLimit),
                                       sizeof(offsetLimit), (int) VALUE_OFFSETLIMITS);
    if(errorCode < 0) {
        _device->disconnect();
        _statusMessage(errorCode);
        return;
    }

    for (unsigned channel = 0; channel < _specification.channels; ++channel) {
        for (unsigned gainID = 0; gainID < _specification.gainLevel.size(); ++gainID) {
            unsigned min = offsetLimit[channel][gainID][0];
            _specification.gainLevel[gainID].offset[channel].setMinChangeEndianess(min);
            unsigned max = offsetLimit[channel][gainID][1];
            _specification.gainLevel[gainID].offset[channel].setMaxChangeEndianess(max);
        }
    }

    setOffset(0,0);
    setOffset(1,0);
    setCoupling(0,DSO::Coupling::DC);
    setCoupling(1,DSO::Coupling::DC);

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

}
