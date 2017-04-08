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

#include "init.h"
#include "hantekDevice.h"
#include "deviceDescriptionEntry.h"
#include "usbCommunication.h"

namespace Hantek2xxx_5xxx {
    DSO::DeviceBase* makeHantekDevice(libusb_device* device, const DSO::DSODeviceDescription& model) {
        return new HantekDevice(std::unique_ptr<DSO::USBCommunication>(new DSO::USBCommunication(device, model)));
    }

    void registerHantek2xxx_5xxxProducts(DSO::DeviceList& devicelist) {
        devicelist.registerModel({"DSO-2090", 0x2090, 0x04b5, 0x02, 0x86, false, makeHantekDevice});
        devicelist.registerModel({"DSO-2150", 0x2150, 0x04b5, 0x02, 0x86, false, makeHantekDevice});
        devicelist.registerModel({"DSO-2250", 0x2250, 0x04b5, 0x02, 0x86, false, makeHantekDevice});
        devicelist.registerModel({"DSO-5200", 0x5200, 0x04b5, 0x02, 0x86, false, makeHantekDevice});
        devicelist.registerModel({"DSO-5200A", 0x520A, 0x04b5, 0x02, 0x86, false, makeHantekDevice});
    }
}
