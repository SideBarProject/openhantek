////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  dso.cpp
//
//  Copyright (C) 2010  Oliver Haag
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


#include <QCoreApplication>

#include "dsoerrorstrings.h"
#include "errorcodes.h"

#include <libusb-1.0/libusb.h>
#include <cerrno>

QString getErrorString(int error) {
    if (error >= LIBUSB_ERROR_OTHER)
        return QString(libusb_strerror((libusb_error)error));
    else
    switch((ErrorCode)error) {
        case ErrorCode::ERROR_WRONG_DRIVER:
            return QCoreApplication::tr("Unknown model");
        case ErrorCode::ERROR_CONNECTION:
            return QCoreApplication::tr("Device not connected or communication error");
        case ErrorCode::ERROR_UNSUPPORTED:
            return QCoreApplication::tr("Not supported by this device");
        case ErrorCode::ERROR_PARAMETER:
            return QCoreApplication::tr("Parameter out of range");
        default:
            return QString();
    }
}
