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

#include <thread>
#include <functional>

#include "protocol.h"
#include "usbCommunication.h"
#include "usbCommunicationQueues.h"
#include "deviceBase.h"
#include "errorcodes.h"

namespace Hantek2xxx_5xxx {

//////////////////////////////////////////////////////////////////////////////
/// Implementation of a DSO DeviceBase for Hantek USB DSO DSO-20xx, DSO-21xx, DSO-22xx, DSO-52xx
class HantekDevice : public DSO::DeviceBase, public DSO::CommunicationThreadQueues {
    public:
        HantekDevice(std::unique_ptr<DSO::USBCommunication> device);
        ~HantekDevice();

        /// Implemented methods from base classes
        virtual ErrorCode setChannelUsed(unsigned int channel, bool used) override;
        virtual ErrorCode setCoupling(unsigned int channel, DSO::Coupling coupling) override;

        virtual unsigned getUniqueID() const override;

        virtual bool needFirmware() const override;
        virtual ErrorCode uploadFirmware() override;

        virtual bool isDeviceConnected() const override;
        virtual void connectDevice() override;
        virtual void disconnectDevice() override;
    private:
        //////////////////////////////////////////////////////////////////////////////
        /// \enum RollState
        /// \brief The states of the roll cycle (Since capture state isn't valid).
        enum class RollState {
            STARTSAMPLING = 0, ///< Start sampling
            ENABLETRIGGER = 1, ///< Enable triggering
            FORCETRIGGER = 2, ///< Force triggering
            GETDATA = 3, ///< Request sample data
            COUNT
        };


        /// \brief Handles all USB communication and sampling until the device gets disconnected.
        void run();
        bool runRollmode(std::vector<unsigned char>& data, RollState& rollState, bool& samplingStarted, unsigned& previouslyReadSamples);
        bool runStandardMode(std::vector<unsigned char>& data, CaptureState& captureState,
                             int& cycleCounter, int& startCycle, int timerIntervall,
                             bool& samplingStarted, DSO::TriggerMode& lastTriggerMode, unsigned& previouslyReadSamples);

        /// Implemented methods from base classes
        virtual double getDownsamplerRate(double bestDownsampler, bool maximum) const override;
        virtual void updatePretriggerPosition(double pretrigger_pos_in_s) override;
        virtual void updateRecordLength(unsigned int index) override;
        virtual void updateSamplerate(DSO::ControlSamplerateLimits* limits, unsigned int downsampler, bool fastRate) override;
        virtual void updateGain(unsigned channel, unsigned char gain, unsigned gainId) override;
        virtual void updateOffset(unsigned int channel, unsigned short int offsetValue) override;
        virtual ErrorCode updateTriggerSource(bool special, unsigned int channel);
        virtual ErrorCode updateTriggerLevel(unsigned int channel, double level);
        virtual ErrorCode updateTriggerSlope(DSO::Slope slope);

        /**
         * @brief Sends a bulk command. _device->bulkWrite cannot be called
         * directly, because a usb control sequence has to be send before each bulk request.
         * This can only be done in the sample thread (in run()).
         * @param command
         * @return Return an usb error code.
         */
        int sampleThreadBulkCommand(USBTransferBuffer* command);

        /// \brief Gets the current state.
        /// This is done in the sample thread (in run())
        /// \return <libusb error code on error (<0) or the trigger position,
        ///          The current CaptureState of the oscilloscope>.
        std::pair<int, CaptureState> readCaptureState();

        /// \brief Gets sample data from the oscilloscope.
        /// This is done in the sample thread (in run())
        /// \param data The data from the oscilloscope is store here.
        /// \return sample count on success, libusb error code on error.
        int readSamples(std::vector<unsigned char>& data, unsigned recordLength, unsigned& previouslyReadSamples);

        /// The USB device for the oscilloscope
        std::unique_ptr<DSO::USBCommunication> _device;
        std::unique_ptr<std::thread> _thread;
        volatile bool _keep_thread_running;

        /// USB device has been disconnected. This will be called if disconnectDevice() is issued before
        /// or if an usb error occured or the device has been plugged out.
        void deviceDisconnected();
};

}
