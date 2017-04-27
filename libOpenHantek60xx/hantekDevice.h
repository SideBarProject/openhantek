////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Declares the Hantek::HantekDevice class.
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

#pragma once

#include <cmath>
#include <limits>
#include <vector>
#include <iostream>
#include <memory>
#include <thread>
#include <functional>
#include <chrono>
#include <future>
#include <sstream>
#include <string>
#include <fstream>

#include "usbCommunication.h"
#include "usbCommunicationQueues.h"
#include "deviceBase.h"
#include "errorcodes.h"
#include "protocol.h"
#include "dsoSettings.h"

class libusb_device;

namespace Hantek60xx {

//////////////////////////////////////////////////////////////////////////////
/// Implementation of a DSO DeviceBase for Hantek USB DSO DSO-60xx
class HantekDevice : public DSO::DeviceBase, public DSO::CommunicationThreadQueues  {
    public:
        HantekDevice(std::unique_ptr<DSO::USBCommunication> device);
        ~HantekDevice();

        virtual unsigned getUniqueID() const override;

        virtual bool needFirmware() const override;
//        virtual ErrorCode uploadFirmware();
        virtual ErrorCode uploadFirmware();

        virtual bool isDeviceConnected() const override;
        virtual void connectDevice() override;
        virtual void disconnectDevice() override;
        ErrorCode resetDevice(bool enable);
    private:
        std::unique_ptr<DSO::USBCommunication> _device;
        std::unique_ptr<std::thread> _thread;
        volatile bool _keep_thread_running;

        /// The DSO samples passes multiple buffers before it appears
        /// on screen. _data is the first one after the usb communication.
        HT6022_DataSize _dataSize = HT6022_DataSize::DS_128KB;
        unsigned char   _data[(int) HT6022_DataSize::DS_1MB*2];

        //////////////////////////////////////////////////////////////////////////////
        /// \enum ControlIndex
        /// \brief The array indices for possible control commands.
        enum ControlIndex {
                CONTROLINDEX_SETOFFSET,
                CONTROLINDEX_SETRELAYS,
                CONTROLINDEX_COUNT
        };

        /// USB device has been disconnected. This will be called if disconnectDevice() is issued before
        /// or if an usb error occured or the device has been plugged out.
        void deviceDisconnected();

        /**
         * @brief Sends a bulk command. _device->bulkWrite cannot be called
         * directly, because a usb control sequence has to be send before each bulk request.
         * This can only be done in the sample thread (in run()).
         * @param command
         * @return Return an usb error code.
         */
        int readSamples();

        /// \brief Handles all USB communication and sampling until the device gets disconnected.
        void run();

        virtual void updatePretriggerPosition(double pretrigger_pos_in_s) override;
        virtual void updateRecordLength(unsigned int index) override;
        virtual ErrorCode updateSamplerate(double timebase);
        virtual ErrorCode updateGain(unsigned channel, unsigned char hwGainCode);
        virtual void updateOffset(unsigned int channel, unsigned short int offsetValue);
        virtual ErrorCode updateTriggerSource(bool special, unsigned int channel);
        virtual ErrorCode updateTriggerLevel(unsigned int channel, double level);
        virtual ErrorCode updateTriggerSlope(DSO::Slope slope);
};

}
