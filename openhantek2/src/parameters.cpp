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
#include "parameters.h"

namespace DsoStrings {
    /// \brief Return string representation of the given channel mode.
    /// \param mode The ::ChannelMode that should be returned as string.
    /// \return The string that should be used in labels etc., empty when invalid.
    QString channelModeString(ChannelMode mode) {
        switch(mode) {
            case CHANNELMODE_VOLTAGE:
                return QCoreApplication::tr("Voltage");
            case CHANNELMODE_SPECTRUM:
                return QCoreApplication::tr("Spectrum");
            default:
                return QString();
        }
    }

    /// \brief Return string representation of the given graph interpolation mode.
    /// \param interpolation The ::InterpolationMode that should be returned as string.
    /// \return The string that should be used in labels etc.
    QString interpolationModeString(ScopeSettings::InterpolationMode interpolation) {
        switch(interpolation) {
            case ScopeSettings::INTERPOLATION_OFF:
                return QCoreApplication::tr("Off");
            case ScopeSettings::INTERPOLATION_LINEAR:
                return QCoreApplication::tr("Linear");
            case ScopeSettings::INTERPOLATION_SINC:
                return QCoreApplication::tr("Sinc");
            default:
                return QString();
        }
    }


    QString graphFormatString(ScopeSettings::GraphFormat format) {
        switch(format) {
            case ScopeSettings::GraphFormat::TY:
                return QCoreApplication::tr("T - Y");
            case ScopeSettings::GraphFormat::XY:
                return QCoreApplication::tr("X - Y");
            default:
                return QString();
        }
    }

}
