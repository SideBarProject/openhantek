////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Hantek device database.
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

#include <string>
#include <functional>

class libusb_device;

namespace DSO {
    class DeviceBase;

    /**
     * An entry for a device database, that can be used by deviceList
     */
    struct DSODeviceDescription {
        std::string modelName; ///< The official product name
        uint16_t    productID; ///< USB product ID
        uint16_t    vendorID;  ///< USB vendor ID
        uint8_t     bulk_endpoint_out; ///< USB endpoint for bulk writing
        uint8_t     bulk_endpoint_in;  ///< USB endpoint for bulk reading
        bool        need_firmware;///< Return true if the device needs firmware to operate
        /// Like the factory pattern. Create an instance of DeviceBase.
        std::function<DeviceBase*(libusb_device*, const DSODeviceDescription&)> createDevice;
    };


}
