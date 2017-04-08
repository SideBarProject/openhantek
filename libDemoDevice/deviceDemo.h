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

namespace DemoDevices {

//////////////////////////////////////////////////////////////////////////////////
/// A demo device has some parameters to be set.
class DeviceDemo {
    public:
        double period_in_s   = 0.100; // 100ms period for a repeating signal
        double random_factor = 0.1;   // 0=no random, 1=full random
        double amplitude_in_V= 1.0;   // Voltage
};

}
