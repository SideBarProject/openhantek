////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Defines various constants, enums and functions for DSO settings.
//
//  Copyright (C) 2010  Oliver Haag <oliver.haag@gmail.com>
//  Copyright (C) 2015  David Graeff <david.graeff@web.de>
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

#include <QString>
#include <QObject>

/// \brief Returns string representation for libusb or other libDSO ErrorCode errors.
/// \param error The error code.
/// \return String explaining the error.
QString getErrorString(int error);

/// Helper class to make the above method available from QML
class ErrorStrings : public QObject {
    Q_OBJECT
public:
    Q_INVOKABLE QString errorString(int error) {
        return getErrorString(error);
    }
};
