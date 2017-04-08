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
#include "dsoSettings.h"
#include "dataAnalyzerSettings.h"

////////////////////////////////////////////////////////////////////////////////
/// \namespace Dso                                                         dso.h
/// \brief All DSO specific things for different modes and so on.
namespace DsoStrings {
    /// \brief Return string representation of the given channel coupling.
    /// \param coupling The ::Coupling that should be returned as string.
    /// \return The string that should be used in labels etc.
    QString couplingString(DSO::Coupling coupling);

    /// \brief Return string representation of the given math mode.
    /// \param mode The ::MathMode that should be returned as string.
    /// \return The string that should be used in labels etc.
    QString mathModeString(DSOAnalyser::MathMode mode);

    /// \brief Return string representation of the given trigger mode.
    /// \param mode The ::TriggerMode that should be returned as string.
    /// \return The string that should be used in labels etc.
    QString triggerModeString(DSO::TriggerMode mode);

    /// \brief Return string representation of the given trigger slope.
    /// \param slope The ::Slope that should be returned as string.
    /// \return The string that should be used in labels etc.
    QString slopeString(DSO::Slope slope);

    /// \brief Return string representation of the given dft window function.
    /// \param window The ::WindowFunction that should be returned as string.
    /// \return The string that should be used in labels etc.
    QString windowFunctionString(DSOAnalyser::WindowFunction window);
}
