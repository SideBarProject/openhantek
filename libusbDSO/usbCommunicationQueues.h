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

#include <vector>
#include <queue>
#include <memory>

#include "utils/transferBuffer.h"
#include "errorcodes.h"

namespace DSO {
class USBCommunication;

//////////////////////////////////////////////////////////////////////////////
/// \brief Part of the base class for an DSO device implementation. To implement
/// thread safety, bulk and control "queues" are established. Those can be "filled"
/// by any thread, and are send within the communication thread via sendCommands(device).
///
/// You can use a transfer buffer in your code like this:
/// ControlSetOffset& control = get<ControlSetOffset>();
/// control.setChannel(channel, offsetValue);
/// addPending(control);
///
/// As soon as you use addPending(..) the TransferBuffer
/// will be added to the queue and the queue is processed in sendPendingCommands(...).
///
/// Implementation details:
/// The compiler ensures that every struct given as template argument
/// to get<T>() will get its own memory, so there is no need to "register" or
/// reserve memory for all used structs.
/// The template specialised get() method will reserve memory for the structure
/// while in compile time.
class CommunicationThreadQueues {
public:
    /// Return an object of the struct of the given type. The state of the object
    /// is kept through the entire lifetime of CommunicationThreadQueues. If you
    /// use an object from get and want it to be reseted to the state after construction,
    /// (e.g. after connect()/disconnect() from a usb device)
    /// you may for example use std::swap in <algorithm>. Example workflow:
    ///
    /// {connect to device}
    /// CoolObject& o = get<CoolObject>();
    /// o.alterState();
    /// addPending(o);
    /// {disconnect from device}
    /// std::swap(o, CoolObject());
    /// {repeat}
    template<class BufferType>
    BufferType& get() {
        static BufferType b;
        return b;
    }

    /// Add pending USBTransferBuffer objects.
    /// \param cmd An object of the type USBTransferBuffer, that also inherits either
    ///        ControlUSB or BulkUSB
    template <class T, class = typename std::is_base_of<USBTransferBuffer, T>::type>
    inline void addPending(const T& cmd) {
        addPending(cmd, tag_type<std::is_base_of<ControlUSB, T>::value,
                            std::is_base_of<BulkUSB, T>::value>());
    }

    /// Clear all queues
    void resetPending() {
        std::queue<const USBTransferBuffer*> a;
        _pendingControlCommands.swap(a);
        _pendingBulkCommands.swap(a);
    }

    /// \brief Sends bulk/control commands directly.
    /// Check if device is connected with isDeviceConnected before.
    /// <p>
    ///        <b>Syntax:</b><br />
    ///        <br />
    ///        Bulk command:
    ///        <pre>send bulk [<em>hex data</em>]</pre>
    ///        %Control command:
    ///        <pre>send control [<em>hex code</em>] [<em>hex data</em>]</pre>
    /// </p>
    /// \param command The command as string (Has to be parsed).
    /// \return See ::ErrorCode::ErrorCode.
    ErrorCode stringCommand(const std::string& command);

    /// Send all pending bulk/control commands
    bool sendPendingCommands(DSO::USBCommunication* device);
protected:
    /// The queue for the usb control commands
    std::queue<const USBTransferBuffer*> _pendingControlCommands;
    /// The queue for the usb bulk commands
    std::queue<const USBTransferBuffer*> _pendingBulkCommands;

    /// A helper struct to differentiate between the tweo addPending overloads.
    template<bool...> struct tag_type {};
    typedef tag_type<true, false> useControlQueue;
    typedef tag_type<false, true> useBulkQueue;

    template <typename T>
    inline void addPending(const T& cmd, useControlQueue) {
        _pendingControlCommands.push(&cmd);
    }

    template <typename T>
    inline void addPending(const T& cmd, useBulkQueue) {
        _pendingBulkCommands.push(&cmd);
    }

};

}
