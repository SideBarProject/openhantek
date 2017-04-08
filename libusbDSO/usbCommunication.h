////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//
/// \copyright (c) 2008, 2009 Oleg Khudyakov <prcoder@potrebitel.ru>
/// \copyright (c) 2010 - 2012 Oliver Haag <oliver.haag@gmail.com>
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
#include "deviceDescriptionEntry.h"
#include "utils/transferBuffer.h"

class libusb_context;
class libusb_device_handle;
class libusb_device;

namespace DSO {

//////////////////////////////////////////////////////////////////////////////
///
/// \brief This class handles the USB communication with the oscilloscope.
class USBCommunication {
    #define USB_COMM_TIMEOUT              500 ///< Timeout for USB transfers in ms
    #define USB_COMM_TIMEOUT_MULTI         10 ///< Timeout for multi packet USB transfers in ms
    #define USB_COMM_ATTEMPTS               3 ///< The number of transfer attempts
    #define USB_COMM_ATTEMPTS_MULTI         1 ///< The number of multi packet transfer attempts

    public:
        /**
         * Create the USBCommunication object but will neither initiate a usb device connection nor initiate libusb.
         */
        USBCommunication(libusb_device *device,
                         const DSODeviceDescription& model);
        ~USBCommunication();

        int connect();
        void disconnect();
        bool isConnected() const;

        // Various methods to handle USB transfers
        int bulkTransfer(unsigned char endpoint, unsigned char *data, unsigned int length, int attempts = USB_COMM_ATTEMPTS, unsigned int timeout = USB_COMM_TIMEOUT);
        int bulkWrite(const unsigned char *data, unsigned int length);
        int bulkRead(unsigned char *data, unsigned int length);
        int bulkReadMulti(unsigned char *data, unsigned int length);

        int controlTransfer(unsigned char type, unsigned char request, unsigned char *data, unsigned int length, int value, int index);
        int controlWrite(unsigned char request, const unsigned char *data, unsigned int length, int value = 0, int index = 0);
        int controlRead(unsigned char request, unsigned char *data, unsigned int length, int value = 0, int index = 0);

        int getPacketSize() const;
        const DSODeviceDescription& model() const;

        uint8_t getUniqueID();
        void setDisconnected_signal(const std::function<void ()>& disconnected_signal);

protected:

        /// The usb context used for this device
        libusb_context *context = 0;
        /// The USB handle for the oscilloscope
        libusb_device_handle *handle = 0;
        /// The USB device for the oscilloscope
        libusb_device *_device;
        /// The number of the claimed interface
        int _interface = -1;
        /// Packet length for the OUT endpoint
        int outPacketLength = 0;
        /// Packet length for the IN endpoint
        int inPacketLength  = 0;

        /// Depending on the usb speed, different bulk sizes are possible. Cache the value.
        int _packetsizeCached;

        /// The model of the connected oscilloscope. Used for the usb endpoints
        /// and to print the model name on stdout.
        const DSODeviceDescription _model;

        std::function<void(void)> _disconnected_signal;
};

}
