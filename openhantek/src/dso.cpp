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


#include <QApplication>
#include <QDebug>
#include "dso.h"

namespace Dso {
	/// \brief Return string representation of the given channel mode.
	/// \param mode The ::ChannelMode that should be returned as string.
	/// \return The string that should be used in labels etc., empty when invalid.
    QString channelModeString(DSO::ChannelMode mode) {
		switch(mode) {
            case DSO::ChannelMode::CHANNELMODE_VOLTAGE:
				return QApplication::tr("Voltage");
            case DSO::ChannelMode::CHANNELMODE_SPECTRUM:
				return QApplication::tr("Spectrum");
			default:
				return QString();
		}
	}
	
	/// \brief Return string representation of the given graph format.
	/// \param format The ::GraphFormat that should be returned as string.
	/// \return The string that should be used in labels etc.
    QString graphFormatString(DSO::GraphFormat format) {
		switch(format) {
            case DSO::GraphFormat::GRAPHFORMAT_TY:
				return QApplication::tr("T - Y");
            case DSO::GraphFormat::GRAPHFORMAT_XY:
				return QApplication::tr("X - Y");
			default:
				return QString();
		}
	}
	
	/// \brief Return string representation of the given channel coupling.
	/// \param coupling The ::Coupling that should be returned as string.
	/// \return The string that should be used in labels etc.
    QString couplingString(DSO::Coupling coupling) {
		switch(coupling) {
            case DSO::Coupling::COUPLING_NONE:
                return QApplication::tr("None");
            case DSO::Coupling::COUPLING_AC:
				return QApplication::tr("AC");
            case DSO::Coupling::COUPLING_DC:
				return QApplication::tr("DC");
            case DSO::Coupling::COUPLING_GND:
				return QApplication::tr("GND");
			default:
				return QString();
		}
	}
	
	/// \brief Return string representation of the given math mode.
	/// \param mode The ::MathMode that should be returned as string.
	/// \return The string that should be used in labels etc.
    QString mathModeString(DSOAnalyzer::MathMode mode) {
		switch(mode) {
            case DSOAnalyzer::MathMode::ADD_CH1_CH2:
                qDebug() << "mathModeString: CH1 + CH2" << endl;
				return QApplication::tr("CH1 + CH2");
            case DSOAnalyzer::MathMode::SUB_CH1_FROM_CH2:
                qDebug() << "mathModeString: CH2 - CH1" << endl;
                return QApplication::tr("CH1 - CH2");
            case DSOAnalyzer::MathMode::SUB_CH2_FROM_CH1:
                qDebug() << "mathModeString: CH2 - CH1" << endl;
				return QApplication::tr("CH2 - CH1");
			default:
				return QString();
		}
	}
	
	/// \brief Return string representation of the given trigger mode.
	/// \param mode The ::TriggerMode that should be returned as string.
	/// \return The string that should be used in labels etc.
    QString triggerModeString(DSO::TriggerMode mode) {
		switch(mode) {
            case DSO::TriggerMode::TRIGGERMODE_AUTO:
				return QApplication::tr("Auto");
            case DSO::TriggerMode::TRIGGERMODE_NORMAL:
				return QApplication::tr("Normal");
            case DSO::TriggerMode::TRIGGERMODE_SINGLE:
				return QApplication::tr("Single");
			default:
				return QString();
		}
	}
	
	/// \brief Return string representation of the given trigger slope.
	/// \param slope The ::Slope that should be returned as string.
	/// \return The string that should be used in labels etc.
    QString slopeString(DSO::Slope slope) {
		switch(slope) {
            case DSO::Slope::SLOPE_POSITIVE:
				return QString::fromUtf8("\u2197");
            case DSO::Slope::SLOPE_NEGATIVE:
				return QString::fromUtf8("\u2198");
			default:
				return QString();
		}
	}
	
	/// \brief Return string representation of the given dft window function.
	/// \param window The ::WindowFunction that should be returned as string.
	/// \return The string that should be used in labels etc.
    QString windowFunctionString(DSOAnalyzer::WindowFunction window) {
		switch(window) {
            case DSOAnalyzer::WindowFunction::WINDOW_RECTANGULAR:
				return QApplication::tr("Rectangular");
            case DSOAnalyzer::WindowFunction::WINDOW_HAMMING:
				return QApplication::tr("Hamming");
            case DSOAnalyzer::WindowFunction::WINDOW_HANN:
				return QApplication::tr("Hann");
            case DSOAnalyzer::WindowFunction::WINDOW_COSINE:
				return QApplication::tr("Cosine");
            case DSOAnalyzer::WindowFunction::WINDOW_LANCZOS:
				return QApplication::tr("Lanczos");
            case DSOAnalyzer::WindowFunction::WINDOW_BARTLETT:
				return QApplication::tr("Bartlett");
            case DSOAnalyzer::WindowFunction::WINDOW_TRIANGULAR:
				return QApplication::tr("Triangular");
            case DSOAnalyzer::WindowFunction::WINDOW_GAUSS:
				return QApplication::tr("Gauss");
            case DSOAnalyzer::WindowFunction::WINDOW_BARTLETTHANN:
				return QApplication::tr("Bartlett-Hann");
            case DSOAnalyzer::WindowFunction::WINDOW_BLACKMAN:
				return QApplication::tr("Blackman");
			//case WINDOW_KAISER:
			//	return QApplication::tr("Kaiser");
            case DSOAnalyzer::WindowFunction::WINDOW_NUTTALL:
				return QApplication::tr("Nuttall");
            case DSOAnalyzer::WindowFunction::WINDOW_BLACKMANHARRIS:
				return QApplication::tr("Blackman-Harris");
            case DSOAnalyzer::WindowFunction::WINDOW_BLACKMANNUTTALL:
				return QApplication::tr("Blackman-Nuttall");
            case DSOAnalyzer::WindowFunction::WINDOW_FLATTOP:
				return QApplication::tr("Flat top");
			default:
				return QString();
		}
	}
	
	/// \brief Return string representation of the given graph interpolation mode.
	/// \param interpolation The ::InterpolationMode that should be returned as string.
	/// \return The string that should be used in labels etc.
    QString interpolationModeString(DSO::InterpolationMode interpolation) {
		switch(interpolation) {
            case DSO::InterpolationMode::INTERPOLATION_OFF:
                return QApplication::tr("Off");
            case DSO::InterpolationMode::INTERPOLATION_LINEAR:
				return QApplication::tr("Linear");
            case DSO::InterpolationMode::INTERPOLATION_SINC:
				return QApplication::tr("Sinc");
			default:
				return QString();
		}
	}
}
