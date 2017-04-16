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


#include <vector>
#include <iostream>
#include <algorithm>
#include <iostream>

#include <libusb-1.0/libusb.h>

#include "usbCommunication.h"

namespace DSO {

////////////////////////////////////////////////////////////////////////////////
/// \brief Initializes the usb things and lists.
/// \param parent The parent widget.
USBCommunication::USBCommunication(libusb_device *device, const DSODeviceDescription& model)
    : _device(device), _model(model) {
}

/// \brief Disconnects the device.
USBCommunication::~USBCommunication() {
    disconnect();
}

uint8_t USBCommunication::getUniqueID() {
    return libusb_get_port_number(_device);
}

void USBCommunication::setDisconnected_signal(const std::function<void ()>& disconnected_signal)
{
    _disconnected_signal = disconnected_signal;
}

/// \brief Search for compatible devices.
/// \return A string with the result of the search.
int USBCommunication::connect() {
    if(handle)
        libusb_close(handle);

    // Open device
    int errorCode = libusb_open(_device, &(handle));
    if(errorCode != LIBUSB_SUCCESS) {
        handle = 0;
        return errorCode;
    }

    libusb_config_descriptor *configDescriptor;
    const libusb_interface *interface;
    const libusb_interface_descriptor *interfaceDescriptor;

    // Search for the needed interface
    libusb_get_config_descriptor(_device, 0, &configDescriptor);
    for(int interfaceIndex = 0; interfaceIndex < (int) configDescriptor->bNumInterfaces; ++interfaceIndex) {
        interface = &configDescriptor->interface[interfaceIndex];
        if(interface->num_altsetting < 1)
            continue;

        interfaceDescriptor = &interface->altsetting[0];
        if(interfaceDescriptor->bInterfaceClass != LIBUSB_CLASS_VENDOR_SPEC ||
            interfaceDescriptor->bInterfaceSubClass != 0 ||
            interfaceDescriptor->bInterfaceProtocol != 0 ||
            interfaceDescriptor->bNumEndpoints != 2)
            continue;

        // That's the interface we need, remove kernel driver
        if(libusb_kernel_driver_active(handle, interfaceDescriptor->bInterfaceNumber) == 1) {
            errorCode = libusb_detach_kernel_driver(handle, interfaceDescriptor->bInterfaceNumber);
            if(errorCode != LIBUSB_SUCCESS)
            {
                libusb_close(handle);
                return errorCode;
            }
        }

        // That's the interface we need, claim it
        errorCode = libusb_claim_interface(handle, interfaceDescriptor->bInterfaceNumber);
        if(errorCode != LIBUSB_SUCCESS) {
            libusb_close(handle);
            handle = 0;
            libusb_free_config_descriptor(configDescriptor);
            return errorCode;
        }

        _interface = interfaceDescriptor->bInterfaceNumber;

        // Check the maximum endpoint packet size, if bulk transfers are required
        if (_model.bulk_endpoint_out || _model.bulk_endpoint_in) {
            const libusb_endpoint_descriptor *endpointDescriptor;
            outPacketLength = 0;
            inPacketLength = 0;
            for (int endpoint = 0; endpoint < interfaceDescriptor->bNumEndpoints; ++endpoint) {
                endpointDescriptor = &(interfaceDescriptor->endpoint[endpoint]);
                if(endpointDescriptor->bEndpointAddress == _model.bulk_endpoint_out) {
                    outPacketLength = endpointDescriptor->wMaxPacketSize;
                } else if(endpointDescriptor->bEndpointAddress == _model.bulk_endpoint_in) {
                    inPacketLength = endpointDescriptor->wMaxPacketSize;
                }
            }
        }
        break;
    }

    libusb_free_config_descriptor(configDescriptor);

    // Store connection speed -> packetsize
    int speed = libusb_get_device_speed(_device);

    switch(speed) {
        case LIBUSB_SPEED_FULL:
            _packetsizeCached = 64;
            break;
        case LIBUSB_SPEED_HIGH:
        case LIBUSB_SPEED_SUPER:
            _packetsizeCached = 512;
            break;
        case LIBUSB_SPEED_UNKNOWN:
        default:
            _packetsizeCached = -1;
            break;
    }
    std::cout << _model.modelName << " successfully connected to USB" << std::endl;
    return LIBUSB_SUCCESS;
}

/// \brief Disconnect the device.
void USBCommunication::disconnect() {
    if(!handle)
        return;

    // Release claimed interface
    libusb_release_interface(handle, _interface);
    _interface = -1;

    // Close device handle
    libusb_close(handle);
    handle = 0;

    _disconnected_signal();
}

/// \brief Check if the oscilloscope is connected.
/// \return true, if a connection is up.
bool USBCommunication::isConnected() const {
    return handle != 0;
}

/// \brief Bulk transfer to/from the oscilloscope.
/// \param endpoint Endpoint number, also sets the direction of the transfer.
/// \param data Buffer for the sent/recieved data.
/// \param length The length of the packet.
/// \param attempts The number of attempts, that are done on timeouts.
/// \param timeout The timeout in ms.
/// \return Number of transferred bytes on success, libusb error code on error.
int USBCommunication::bulkTransfer(unsigned char endpoint, unsigned char *data, unsigned int length, int attempts, unsigned int timeout) {
    if(!handle)
        return LIBUSB_ERROR_NO_DEVICE;

    int errorCode = LIBUSB_ERROR_TIMEOUT;
    int transferred;
    for(int attempt = 0; (attempt < attempts || attempts == -1) && errorCode == LIBUSB_ERROR_TIMEOUT; ++attempt)
            errorCode = libusb_bulk_transfer(handle, endpoint, data, length, &transferred, timeout);

    if(errorCode == LIBUSB_ERROR_NO_DEVICE)
        disconnect();

    if(errorCode < 0)
        return errorCode;
    else
        return transferred;
}

/// \brief Bulk write to the oscilloscope.
/// \param data Buffer for the sent/recieved data.
/// \param length The length of the packet.
/// \param attempts The number of attempts, that are done on timeouts.
/// \return Number of sent bytes on success, libusb error code on error.
int USBCommunication::bulkWrite(const unsigned char *data, unsigned int length) {
    return bulkTransfer(_model.bulk_endpoint_out, (unsigned char *)data, length, USB_COMM_ATTEMPTS);
}

/// \brief Bulk read from the oscilloscope.
/// \param data Buffer for the sent/recieved data.
/// \param length The length of the packet.
/// \param attempts The number of attempts, that are done on timeouts.
/// \return Number of received bytes on success, libusb error code on error.
int USBCommunication::bulkRead(unsigned char *data, unsigned int length) {
    return bulkTransfer(_model.bulk_endpoint_in, data, length, USB_COMM_ATTEMPTS);
}

/// \brief Multi packet bulk read from the oscilloscope.
/// \param data Buffer for the sent/recieved data.
/// \param length The length of data contained in the packets.
/// \param attempts The number of attempts, that are done on timeouts.
/// \return Number of received bytes on success, libusb error code on error.
int USBCommunication::bulkReadMulti(unsigned char *data, unsigned int length) {
    unsigned received = 0;
    while(received < length) {
        unsigned read_size = std::min(length - received, (unsigned) inPacketLength);
        int errorCode = bulkTransfer(_model.bulk_endpoint_in, data, read_size, USB_COMM_ATTEMPTS_MULTI, USB_COMM_TIMEOUT_MULTI);
        if(errorCode <= 0)
            return errorCode;
        data += read_size;
        received += errorCode;
    }

    return received;
}

/// \brief Control transfer to the oscilloscope.
/// \param type The request type, also sets the direction of the transfer.
/// \param request The request field of the packet.
/// \param data Buffer for the sent/recieved data.
/// \param length The length field of the packet.
/// \param value The value field of the packet.
/// \param index The index field of the packet.
/// \param attempts The number of attempts, that are done on timeouts.
/// \return Number of transferred bytes on success, libusb error code on error.
int USBCommunication::controlTransfer(unsigned char type, unsigned char request, unsigned char *data, unsigned int length, int value, int index) {
    if(!handle) {
        std::cout << "USBCommunication::controlTransfer: no device" << std::endl;
        return LIBUSB_ERROR_NO_DEVICE;
    }
    int attempts = USB_COMM_ATTEMPTS;
    int errorCode = LIBUSB_ERROR_TIMEOUT;

    std::cout << "USBCommunication::controlTransfer: " << std::endl;
    std::cout << "Request Type: 0x" << std::hex << (int)type << std::endl;
    std::cout << "Request: 0x" << std::hex << (int) request << std::endl;
    std::cout << "value: " << value << std::endl;
    std::cout << "index: " << index << std::endl;
    std::cout << "data: " << (int)*data << std::endl;
    std::cout << "length: " << length << std::endl;
    for(int attempt = 0; (attempt < attempts || attempts == -1) && errorCode == LIBUSB_ERROR_TIMEOUT; ++attempt)
        errorCode = libusb_control_transfer(handle, type, request, value, index, data, length, USB_COMM_TIMEOUT);

    if(errorCode == LIBUSB_ERROR_NO_DEVICE)
        disconnect();
    return errorCode;
}

/// \brief Control write to the oscilloscope.
/// \param request The request field of the packet.
/// \param data Buffer for the sent/recieved data.
/// \param length The length field of the packet.
/// \param value The value field of the packet.
/// \param index The index field of the packet.
/// \param attempts The number of attempts, that are done on timeouts.
/// \return Number of sent bytes on success, libusb error code on error.
int USBCommunication::controlWrite(unsigned char request, const unsigned char *data, unsigned int length, int value, int index) {
    return controlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, request,  (unsigned char *)data, length, value, index);
}

/// \brief Control read to the oscilloscope.
/// \param request The request field of the packet.
/// \param data Buffer for the sent/recieved data.
/// \param length The length field of the packet.
/// \param value The value field of the packet.
/// \param index The index field of the packet.
/// \param attempts The number of attempts, that are done on timeouts.
/// \return Number of received bytes on success, libusb error code on error.
int USBCommunication::controlRead(unsigned char request, unsigned char *data, unsigned int length, int value, int index) {
    return controlTransfer(LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, request, data, length, value, index);
}

/// \brief Gets the maximum size of one packet transmitted via bulk transfer.
/// \return The maximum packet size in bytes, -1 on error.
int USBCommunication::getPacketSize() const {
    return _packetsizeCached;
}

const DSODeviceDescription& USBCommunication::model() const { return _model; }

}
