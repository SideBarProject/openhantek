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
#include <chrono>
#include <future>

#include "deviceBase.h"
#include "devicedummy.h"
#include "deviceDemo.h"
#include "errorcodes.h"

namespace DemoDevices {

//////////////////////////////////////////////////////////////////////////////////
/// Implementation of a DSO DeviceBase for a demo device which outputs a sine wave
class SineWaveDevice : public DeviceDummy, DeviceDemo  {
    public:
        SineWaveDevice();
        ~SineWaveDevice();

        virtual unsigned getUniqueID() const override;

        virtual bool needFirmware() const override;
        virtual ErrorCode uploadFirmware() override;

        virtual bool isDeviceConnected() const override;
        virtual void connectDevice() override;
        virtual void disconnectDevice() override;
    private:
        std::unique_ptr<std::thread> _thread;
        volatile bool _keep_thread_running;

        /// \brief Generates the sine wave samples
        void run();
};

}
