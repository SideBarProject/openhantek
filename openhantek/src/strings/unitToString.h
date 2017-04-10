////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Provides miscellaneous helper functions.
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

#pragma once
#include <QString>
#include <QTime>

namespace UnitToString {
    //////////////////////////////////////////////////////////////////////////////
    /// \enum Unit                                                        helper.h
    /// \brief The various units supported by valueToString.
    enum Unit {
        UNIT_VOLTS, UNIT_DECIBEL,
        UNIT_SECONDS, UNIT_HERTZ,
        UNIT_SAMPLES, UNIT_COUNT
    };

    QString valueToString(double value, Unit unit, int precision = -1);

};
