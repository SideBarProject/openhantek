////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Declares the DSO::DeviceBase class.
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

#include <functional>
#include <vector>
#include "deviceBaseSamples.h"
#include "deviceBaseSpecifications.h"
#include "dsoSpecification.h"
#include "dsoSettings.h"
#include "errorcodes.h"

namespace DSO {

//////////////////////////////////////////////////////////////////////////////
/// \brief The base class for an DSO device implementation. It inherits from all
///        deviceBase* classes (Commands, Trigger, Samples, Specifications).
class DeviceBase : public DeviceBaseSamples {
    public:
        DeviceBase(const DSODeviceDescription& model) : DeviceBaseSamples(model) {}

        /// \brief Enables/disables filtering of the given channel.
        /// \param channel The channel that should be set.
        /// \param used true if the channel should be sampled.
        /// \return See ::ErrorCode::ErrorCode.
        virtual ErrorCode setChannelUsed(unsigned int channel, bool used);

        /// \brief Set the coupling (AC/DC/Off) for the given channel.
        /// \param channel The channel that should be set.
        /// \param coupling The new coupling for the channel.
        /// \return See ::ErrorCode::ErrorCode.this->couplingStrings.append(Dso::couplingString((DSO::Coupling) DSO::Coupling::COUPLING_DC));
        virtual ErrorCode setCoupling(unsigned int channel, Coupling coupling);

        /// \brief Sets the gain for the given channel.
        /// Find lowest gain voltage in DeviceBaseSpecifications::gainLevel
        /// thats at least as high as the requested.
        /// \param channel The channel that should be set.
        /// \param gain The gain that should be met (V/div).
        /// \return Returns ::ErrorCode::ERROR_PARAMETER if channel or gain
        ///         level not found,
        virtual ErrorCode setGain(unsigned int channel, double gain);

        double getGainIndex(unsigned int channel);

        /// \brief Set the offset for the given channel.
        /// \param channel The channel that should be set.
        /// \param offset The new offset value (0.0 - 1.0).
        /// \return See ::ErrorCode::ErrorCode.this->couplingStrings.append(Dso::couplingString((DSO::Coupling) DSO::Coupling::COUPLING_DC));
        virtual ErrorCode setOffset(unsigned int channel, double offset);

        /// \brief Set the trigger source.
        /// \param special true for a special channel (EXT, ...) as trigger source.
        /// \param id The number of the channel, that should be used as trigger.
        /// \return See ::ErrorCode::ErrorCode.
        virtual ErrorCode setTriggerSource(bool special, unsigned int channel);

        /// \brief Set the trigger level.
        /// \param channel The channel that should be set.
        /// \param level The new trigger level (V).
        /// \return See ::ErrorCode::ErrorCode.
        virtual ErrorCode setTriggerLevel(unsigned int channel, double level);

        /// \brief Set the trigger slope.
        /// \param slope The Slope that should cause a trigger.
        /// \return See ::ErrorCode::ErrorCode.
        virtual ErrorCode setTriggerSlope(Slope slope);

        /// \brief Set the trigger mode.
        /// \return See ::ErrorCode::ErrorCode.
        ErrorCode setTriggerMode(TriggerMode mode);

        /// \brief Set the trigger mode.
        /// \return True if a firmware upload is necessary. connectDevice() will fail if this
        ///         method return true.
        virtual bool needFirmware() const = 0;

        void setFirmwareFilename(std::string filename);

        /// \brief Upload firmware to the DSO. If no firmware is required (needFirmware==false)
        ///        this method will return ERROR_NONE. It can only be called if no connection
        ///        to the device has been made so far (isDeviceConnected==false) otherwise returns
        ///        ERROR_UNSUPPORTED. This method should be asynchronous and upload firmware in another
        ///        thread with a default timeout of 60 sec. It should report about the status via the
        ///        _uploadProgress signal around every second if the upload duration is longer than 5 sec.
        /// \return See ::ErrorCode::ErrorCode.
        virtual ErrorCode uploadFirmware() = 0;

        /// \brief Return special trigger sources
        /// \return Special trigger sources
        const std::vector<std::string>& getSpecialTriggerSources() const;

        /// \brief A unique id that is important for the DeviceList to identify already
        /// connected devices. For usb devices this is the bus/slot number otherwise
        /// preferably a hash of the model name.
        virtual unsigned getUniqueID() const = 0;

        /// \return Return true if a connection to the device (e.g. usb device) is established.
        virtual bool isDeviceConnected() const = 0;

        /// \brief Try to connect to the oscilloscope. For usb devices this will open the usb interface.
        virtual void connectDevice() = 0;

        /// \brief Disconnect the oscilloscope.
        virtual void disconnectDevice() = 0;

        /// \brief Returns specs for gain settings available on the device
        const std::vector<DSO::dsoGainLevel>& getGainSpecs() const;

        /// \brief Returns the available coupling for the model
        const std::vector<DSO::Coupling> &getAvailableCoupling() const;

        /// \brief Returns the no of physical channels
        unsigned int getPhysicalChannels();

        /// \brief Returns the sampling rates supported by the device
        const std::vector<DSO::dsoAvailableSamplingRate> &getAvailableSamplingrates() const;

        /// \brief Returns the record lengths supported by the device
        const std::vector<DSO::HWRecordLengthID> &getRecordLength() const;

        /// \brief return sampling rate from timebase
        double getSamplingrateFromTimebase(double timebase);

        /// \brief return record length from timebase
        HWRecordLengthID getRecordLengthFromTimebase(double timebase);

        /// \brief return current downsampling rate
        virtual int getDownsamplerRateFromTimebase(double timebase);

        /// \brief set timebase
        ErrorCode setTimebase(double timebase);

        /// \brief set timebase
        double getTimebase();

    /**
     * This section contains callback methods. Register your function or class method to get notified
     * of events.
     */

        /// The oscilloscope device has been connected
        std::function<void(void)> _deviceConnected = [](){};
        /// The oscilloscope device has been disconnected
        std::function<void(void)> _deviceDisconnected = [](){};
        /// Upload progress. Parameter is progress in percentage: 100% == completed.
        /// Sender is not in the main thread! If progress is < 0 then it represents an ::ErrorCode::ErrorCode.
        /// Return true if you want to continue the upload, false if you want to abort it.
        std::function<bool(int)> _uploadProgress = [](int){return true;};
        /// Status message about the oscilloscope (int messageID)
        std::function<void(int)> _statusMessage = [](int){};


protected:
        /// \brief Reset settings to a reasonable default state.
        /// Should be called by connectDevice().
        void resetSettings();

        /// \brief Sets the gain for the given channel.
        /// \param channel The channel that should be set.
        /// \param gain The gain that should be met (V/div).
        virtual ErrorCode updateGain(unsigned channel, unsigned char gainIndex) = 0;

        /// \brief Set the offset for the given channel.
        /// \param channel The channel that should be set.
        /// \param offset The new offset value (0.0 - 1.0).
        virtual void updateOffset(unsigned int channel, unsigned short int offsetValue) = 0;

        /// \brief Set the trigger source.
        /// \param special true for a special channel (EXT, ...) as trigger source.
        /// \param id The number of the channel, that should be used as trigger.
        virtual ErrorCode updateTriggerSource(bool special, unsigned int channel) = 0;

        /// \brief Set the trigger level.
        /// \param channel The channel that should be set.
        /// \param level The new trigger level (V).
        virtual ErrorCode updateTriggerLevel(unsigned int channel, double level) = 0;

        /// \brief Set the trigger slope.
        /// \param slope The Slope that should cause a trigger.
        virtual ErrorCode updateTriggerSlope(Slope slope) = 0;
};

}
