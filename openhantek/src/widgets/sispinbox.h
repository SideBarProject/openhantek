////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Declares the SiSpinBox class.
//
//  Copyright (C) 2010, 2011  Oliver Haag
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


#ifndef SISPINBOX_H
#define SISPINBOX_H


#include <QDoubleSpinBox>
#include <QStringList>
#include "unitToString.h"

////////////////////////////////////////////////////////////////////////////////
///
/// \brief A spin box with SI prefix support.
/// This spin box supports the SI prefixes (k/M/G/T) after its value and allows
/// floating point values. The step size is increasing in an exponential way, to
/// keep the percentual difference between the steps at equal levels.
class SiSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

    public:
        explicit SiSpinBox(QWidget *parent = 0);
        SiSpinBox(UnitToString::Unit unit, QWidget *parent = 0);
        ~SiSpinBox();

        QValidator::State validate(QString &input, int &pos) const;
        /// \brief Converts string containing value and (prefix+)unit to double (Counterpart to Helper::valueToString).
        /// \param text The text containing the value and its unit.
        /// \param unit The base unit of the value.
        /// \param ok Pointer to a success-flag, true on success, false on error.
        /// \return Decoded value.
        double valueFromText(const QString &text, bool* ok = 0) const;
        QString textFromValue(double val) const;
        void fixup(QString &input) const;
        void stepBy(int steps);
        bool setUnit(UnitToString::Unit unit);
        void setUnitPostfix(const QString &postfix);
        void setSteps(const QList<double> &steps);


    private:
        void init();

        UnitToString::Unit unit; ///< The SI unit used for this spin box
        QString unitPostfix; ///< Shown after the unit
        QList<double> steps; ///< The steps, begins from start after last element

        bool steppedTo; ///< true, if the current value was reached using stepBy
        int stepId; ///< The index of the last step reached using stepBy

    private slots:
        void resetSteppedTo();
};

#endif
