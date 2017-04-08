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

namespace Hantek60xx {
    DSO::DeviceBase* makeHantek60xxDevice(libusb_device* device, const DSO::DSODeviceDescription& model) {
        HantekDevice* h = new HantekDevice(std::unique_ptr<DSO::USBCommunication>(new DSO::USBCommunication(device, model)));
        return h;
    }

    void registerHantek60xxProducts(DSO::DeviceList& devicelist) {
        devicelist.registerModel({"DSO-HT6022BE", 0X6022, 0x04b5, 0, 0x86, false, makeHantek60xxDevice});
        devicelist.registerModel({"DSO-HT6022BE", 0X6022, 0x04b4, 0, 0, true,  makeHantek60xxDevice});
        devicelist.registerModel({"DSO-HT6022BL", 0X602A, 0x04b5, 0, 0x86, false, makeHantek60xxDevice});
        devicelist.registerModel({"DSO-HT6022BL", 0X602A, 0x04b4, 0, 0, true,  makeHantek60xxDevice});
    }
}
