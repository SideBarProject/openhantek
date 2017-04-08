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
#include "dsostrings.h"

namespace DsoStrings {
    using namespace DSOAnalyser;
    using namespace DSO;

    QString couplingString(Coupling coupling) {
        switch(coupling) {
            case Coupling::AC:
                return QCoreApplication::tr("AC");
            case Coupling::DC:
                return QCoreApplication::tr("DC");
            case Coupling::GND:
                return QCoreApplication::tr("GND");
            default:
                return QString();
        }
    }

    QString mathModeString(DSOAnalyser::MathMode mode) {
        switch(mode) {
            case MathMode::ADD_CH1_CH2:
                return QCoreApplication::tr("CH1 + CH2");
            case MathMode::SUB_CH2_FROM_CH1:
                return QCoreApplication::tr("CH1 - CH2");
            case MathMode::SUB_CH1_FROM_CH2:
                return QCoreApplication::tr("CH2 - CH1");
            default:
                return QString();
        }
    }

    QString triggerModeString(TriggerMode mode) {
        switch(mode) {
            case TriggerMode::AUTO:
                return QCoreApplication::tr("Auto");
            case TriggerMode::NORMAL:
                return QCoreApplication::tr("Normal");
            case TriggerMode::SINGLE:
                return QCoreApplication::tr("Single");
            default:
                return QString();
        }
    }

    QString slopeString(Slope slope) {
        switch(slope) {
            case Slope::POSITIVE:
                return QString::fromUtf8("\u2197");
            case Slope::NEGATIVE:
                return QString::fromUtf8("\u2198");
            default:
                return QString();
        }
    }

    QString windowFunctionString(WindowFunction window) {
        switch(window) {
            case WINDOW_RECTANGULAR:
                return QCoreApplication::tr("Rectangular");
            case WINDOW_HAMMING:
                return QCoreApplication::tr("Hamming");
            case WINDOW_HANN:
                return QCoreApplication::tr("Hann");
            case WINDOW_COSINE:
                return QCoreApplication::tr("Cosine");
            case WINDOW_LANCZOS:
                return QCoreApplication::tr("Lanczos");
            case WINDOW_BARTLETT:
                return QCoreApplication::tr("Bartlett");
            case WINDOW_TRIANGULAR:
                return QCoreApplication::tr("Triangular");
            case WINDOW_GAUSS:
                return QCoreApplication::tr("Gauss");
            case WINDOW_BARTLETTHANN:
                return QCoreApplication::tr("Bartlett-Hann");
            case WINDOW_BLACKMAN:
                return QCoreApplication::tr("Blackman");
            //case WINDOW_KAISER:
            //    return QCoreApplication::tr("Kaiser");
            case WINDOW_NUTTALL:
                return QCoreApplication::tr("Nuttall");
            case WINDOW_BLACKMANHARRIS:
                return QCoreApplication::tr("Blackman-Harris");
            case WINDOW_BLACKMANNUTTALL:
                return QCoreApplication::tr("Blackman-Nuttall");
            case WINDOW_FLATTOP:
                return QCoreApplication::tr("Flat top");
            default:
                return QString();
        }
    }
}
