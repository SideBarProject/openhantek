////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  helper.cpp
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


#include <cmath>

#include <QApplication>
#include <QLocale>
#include <QStringList>

#include "unitToString.h"

namespace UnitToString {
    /// \brief Converts double to string containing value and (prefix+)unit (Counterpart to Helper::stringToValue).
    /// \param value The value in prefixless units.
    /// \param unit The unit for the value.
    /// \param precision Significant digits, 0 for integer, -1 for auto.
    /// \return String with the value and unit.
    QString valueToString(double value, Unit unit, int precision) {
        char format = (precision < 0) ? 'g' : 'f';

        switch(unit) {
            case UNIT_VOLTS: {
                // Voltage string representation
                int logarithm = floor(log10(fabs(value)));
                if(value < 1e-3)
                    return QApplication::tr("%L1 µV").arg(value / 1e-6, 0, format, (precision <= 0) ? precision : qBound(0, precision - 7 - logarithm, precision));
                else if(value < 1.0)
                    return QApplication::tr("%L1 mV").arg(value / 1e-3, 0, format, (precision <= 0) ? precision : (precision - 4 - logarithm));
                else
                    return QApplication::tr("%L1 V").arg(value, 0, format, (precision <= 0) ? precision : qMax(0, precision - 1 - logarithm));
            }
            case UNIT_DECIBEL:
                // Power level string representation
                return QApplication::tr("%L1 dB").arg(value, 0, format, (precision <= 0) ? precision : qBound(0, precision - 1 - (int) floor(log10(fabs(value))), precision));

            case UNIT_SECONDS:
                // Time string representation
                if(value < 1e-9)
                    return QApplication::tr("%L1 ps").arg(value / 1e-12, 0, format, (precision <= 0) ? precision : qBound(0, precision - 13 - (int) floor(log10(fabs(value))), precision));
                else if(value < 1e-6)
                    return QApplication::tr("%L1 ns").arg(value / 1e-9, 0, format, (precision <= 0) ? precision : (precision - 10 - (int) floor(log10(fabs(value)))));
                else if(value < 1e-3)
                    return QApplication::tr("%L1 µs").arg(value / 1e-6, 0, format, (precision <= 0) ? precision : (precision - 7 - (int) floor(log10(fabs(value)))));
                else if(value < 1.0)
                    return QApplication::tr("%L1 ms").arg(value / 1e-3, 0, format, (precision <= 0) ? precision : (precision - 4 - (int) floor(log10(fabs(value)))));
                else if(value < 60)
                    return QApplication::tr("%L1 s").arg(value, 0, format, (precision <= 0) ? precision : (precision - 1 - (int) floor(log10(fabs(value)))));
                else if(value < 3600)
                    return QApplication::tr("%L1 min").arg(value / 60, 0, format, (precision <= 0) ? precision : (precision - 1 - (int) floor(log10(value / 60))));
                else
                    return QApplication::tr("%L1 h").arg(value / 3600, 0, format, (precision <= 0) ? precision : qMax(0, precision - 1 - (int) floor(log10(value / 3600))));

            case UNIT_HERTZ: {
                // Frequency string representation
                int logarithm = floor(log10(fabs(value)));
                if(value < 1e3)
                    return QApplication::tr("%L1 Hz").arg(value, 0, format, (precision <= 0) ? precision : qBound(0, precision - 1 - logarithm, precision));
                else if(value < 1e6)
                    return QApplication::tr("%L1 kHz").arg(value / 1e3, 0, format, (precision <= 0) ? precision : precision + 2 - logarithm);
                else if(value < 1e9)
                    return QApplication::tr("%L1 MHz").arg(value / 1e6, 0, format, (precision <= 0) ? precision : precision + 5 - logarithm);
                else
                    return QApplication::tr("%L1 GHz").arg(value / 1e9, 0, format, (precision <= 0) ? precision : qMax(0, precision + 8 - logarithm));
            }
            case UNIT_SAMPLES: {
                // Sample count string representation
                int logarithm = floor(log10(fabs(value)));
                if(value < 1e3)
                    return QApplication::tr("%L1 S").arg(value, 0, format, (precision <= 0) ? precision : qBound(0, precision - 1 - logarithm, precision));
                else if(value < 1e6)
                    return QApplication::tr("%L1 kS").arg(value / 1e3, 0, format, (precision <= 0) ? precision : precision + 2 - logarithm);
                else if(value < 1e9)
                    return QApplication::tr("%L1 MS").arg(value / 1e6, 0, format, (precision <= 0) ? precision : precision + 5 - logarithm);
                else
                    return QApplication::tr("%L1 GS").arg(value / 1e9, 0, format, (precision <= 0) ? precision : qMax(0, precision + 8 - logarithm));
            }
            default:
                return QString();
        }
    }
}
