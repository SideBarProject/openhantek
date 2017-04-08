////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  dockwindows.cpp
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


#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDockWidget>
#include <QLabel>


#include "dockwindows.h"

#include "settings.h"
#include "sispinbox.h"
#include "unitToString.h"
#include "dsostrings.h"

#include "deviceBase.h"

////////////////////////////////////////////////////////////////////////////////
// class HorizontalDock
/// \brief Initializes the horizontal axis docking window.
/// \param settings The target settings object.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
HorizontalDock::HorizontalDock(OpenHantekSettings *settings, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(tr("Horizontal"), parent, flags) {
    this->settings = settings;

    // Initialize elements
    this->samplerateLabel = new QLabel(tr("Samplerate"));
    this->samplerateSiSpinBox = new SiSpinBox(UnitToString::UNIT_SAMPLES);
    this->samplerateSiSpinBox->setMinimum(1);
    this->samplerateSiSpinBox->setMaximum(1e8);
    this->samplerateSiSpinBox->setUnitPostfix("/s");

    QList<double> timebaseSteps;
    timebaseSteps << 1.0 << 2.0 << 4.0 << 10.0;

    this->timebaseLabel = new QLabel(tr("Timebase"));
    this->timebaseSiSpinBox = new SiSpinBox(UnitToString::UNIT_SECONDS);
    this->timebaseSiSpinBox->setSteps(timebaseSteps);
    this->timebaseSiSpinBox->setMinimum(1e-9);
    this->timebaseSiSpinBox->setMaximum(3.6e3);

    this->frequencybaseLabel = new QLabel(tr("Frequencybase"));
    this->frequencybaseSiSpinBox = new SiSpinBox(UnitToString::UNIT_HERTZ);
    this->frequencybaseSiSpinBox->setMinimum(1.0);
    this->frequencybaseSiSpinBox->setMaximum(100e6);

    this->recordLengthLabel = new QLabel(tr("Record length"));
    this->recordLengthComboBox = new QComboBox();

    this->formatLabel = new QLabel(tr("Format"));
    this->formatComboBox = new QComboBox();
    for(int format = (int)GraphFormat::TY; format < (int)GraphFormat::COUNT; ++format)
        this->formatComboBox->addItem(DsoStrings::graphFormatString((GraphFormat) format));

    this->dockLayout = new QGridLayout();
    this->dockLayout->setColumnMinimumWidth(0, 64);
    this->dockLayout->setColumnStretch(1, 1);
    this->dockLayout->addWidget(this->samplerateLabel, 0, 0);
    this->dockLayout->addWidget(this->samplerateSiSpinBox, 0, 1);
    this->dockLayout->addWidget(this->timebaseLabel, 1, 0);
    this->dockLayout->addWidget(this->timebaseSiSpinBox, 1, 1);
    this->dockLayout->addWidget(this->frequencybaseLabel, 2, 0);
    this->dockLayout->addWidget(this->frequencybaseSiSpinBox, 2, 1);
    this->dockLayout->addWidget(this->recordLengthLabel, 3, 0);
    this->dockLayout->addWidget(this->recordLengthComboBox, 3, 1);
    this->dockLayout->addWidget(this->formatLabel, 4, 0);
    this->dockLayout->addWidget(this->formatComboBox, 4, 1);

    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    this->dockWidget = new QWidget();
    this->dockWidget->setLayout(this->dockLayout);
    this->setWidget(this->dockWidget);

    // Connect signals and slots
    connect(this->samplerateSiSpinBox, static_cast<void (SiSpinBox::*)(double)>(&SiSpinBox::valueChanged),
            this, &HorizontalDock::samplerateSelected);
    connect(this->timebaseSiSpinBox, static_cast<void (SiSpinBox::*)(double)>(&SiSpinBox::valueChanged),
            this, &HorizontalDock::timebaseSelected);
    connect(this->frequencybaseSiSpinBox, static_cast<void (SiSpinBox::*)(double)>(&SiSpinBox::valueChanged),
            this, &HorizontalDock::frequencybaseSelected);
    connect(this->recordLengthComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &HorizontalDock::recordLengthSelected);
    connect(this->formatComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &HorizontalDock::formatSelected);

    // Set values
    this->setSamplerate(this->settings->scope.horizontal.samplerate);
    this->setTimebase(this->settings->scope.horizontal.timebase);
    this->setFrequencybase(this->settings->scope.horizontal.frequencybase);
    this->setRecordLength(this->settings->scope.horizontal.recordLength);
    this->setFormat(this->settings->scope.horizontal.format);
}

/// \brief Don't close the dock, just hide it.
/// \param event The close event that should be handled.
void HorizontalDock::closeEvent(QCloseEvent *event) {
    this->hide();

    event->accept();
}

/// \brief Changes the frequencybase.
/// \param frequencybase The frequencybase in hertz.
void HorizontalDock::setFrequencybase(double frequencybase) {
    this->suppressSignals = true;
    this->frequencybaseSiSpinBox->setValue(frequencybase);
    this->suppressSignals = false;
}

/// \brief Changes the samplerate.
/// \param samplerate The samplerate in seconds.
void HorizontalDock::setSamplerate(double samplerate) {
    this->suppressSignals = true;
    this->samplerateSiSpinBox->setValue(samplerate);
    this->suppressSignals = false;
}

/// \brief Changes the timebase.
/// \param timebase The timebase in seconds.
void HorizontalDock::setTimebase(double timebase) {
    this->suppressSignals = true;
    this->timebaseSiSpinBox->setValue(timebase);
    this->suppressSignals = false;
}

/// \brief Changes the record length if the new value is supported.
/// \param recordLength The record length in samples.
void HorizontalDock::setRecordLength(unsigned int recordLength) {
    int index = this->recordLengthComboBox->findData(recordLength);

    if(index != -1) {
        this->suppressSignals = true;
        this->recordLengthComboBox->setCurrentIndex(index);
        this->suppressSignals = false;
    }
}

/// \brief Changes the format if the new value is supported.
/// \param format The format for the horizontal axis.
/// \return Index of format-value, -1 on error.
int HorizontalDock::setFormat(GraphFormat format) {
    if(format >= GraphFormat::TY && format <= GraphFormat::XY) {
        this->suppressSignals = true;
        this->formatComboBox->setCurrentIndex((int)format);
        this->suppressSignals = false;
        return (int)format;
    }

    return -1;
}

/// \brief Updates the available record lengths in the combo box.
/// \param recordLengths The available record lengths for the combo box.
void HorizontalDock::availableRecordLengthsChanged(const std::vector<unsigned> &recordLengths, unsigned long) {
    /// \todo Empty lists should be interpreted as scope supporting continuous record length values.
    this->recordLengthComboBox->blockSignals(true); // Avoid messing up the settings
    this->recordLengthComboBox->setUpdatesEnabled(false);

    // Update existing elements to avoid unnecessary index updates
    unsigned index = 0;
    for(; index < recordLengths.size(); ++index) {
        unsigned int recordLengthItem = recordLengths[index];
        if(index < (unsigned)this->recordLengthComboBox->count()) {
            this->recordLengthComboBox->setItemData(index, recordLengthItem);
            this->recordLengthComboBox->setItemText(index, recordLengthItem == UINT_MAX ? tr("Roll") : UnitToString::valueToString(recordLengthItem, UnitToString::UNIT_SAMPLES, 3));
        }
        else {
            this->recordLengthComboBox->addItem(recordLengthItem == UINT_MAX ? tr("Roll") : UnitToString::valueToString(recordLengthItem, UnitToString::UNIT_SAMPLES, 3), (uint) recordLengthItem);
        }
    }
    // Remove extra elements
    for(int extraIndex = this->recordLengthComboBox->count() - 1; extraIndex > (int)index; --extraIndex) {
        this->recordLengthComboBox->removeItem(extraIndex);
    }

    this->setRecordLength(this->settings->scope.horizontal.recordLength);
    this->recordLengthComboBox->setUpdatesEnabled(true);
    this->recordLengthComboBox->blockSignals(false);
}

/// \brief Updates the minimum and maximum of the samplerate spin box.
/// \param minimum The minimum value the spin box should accept.
/// \param maximum The minimum value the spin box should accept.
void HorizontalDock::samplerateLimitsChanged(double minimum, double maximum) {
    this->suppressSignals = true;
    this->samplerateSiSpinBox->setMinimum(minimum);
    this->samplerateSiSpinBox->setMaximum(maximum);
    this->suppressSignals = false;
}

/// \brief Called when the frequencybase spinbox changes its value.
/// \param frequencybase The frequencybase in hertz.
void HorizontalDock::frequencybaseSelected(double frequencybase) {
    this->settings->scope.horizontal.frequencybase = frequencybase;
    if(!this->suppressSignals)
        emit frequencybaseChanged(frequencybase);
}

/// \brief Called when the samplerate spinbox changes its value.
/// \param samplerate The samplerate in samples/second.
void HorizontalDock::samplerateSelected(double samplerate) {
    this->settings->scope.horizontal.samplerate = samplerate;
    if(!this->suppressSignals) {
        this->settings->scope.horizontal.samplerateSet = true;
        emit samplerateChanged(samplerate);
    }
}

/// \brief Called when the timebase spinbox changes its value.
/// \param timebase The timebase in seconds.
void HorizontalDock::timebaseSelected(double timebase) {
    this->settings->scope.horizontal.timebase = timebase;
    if(!this->suppressSignals) {
        this->settings->scope.horizontal.samplerateSet = false;
        emit timebaseChanged(timebase);
    }
}

/// \brief Called when the record length combo box changes its value.
/// \param index The index of the combo box item.
void HorizontalDock::recordLengthSelected(int index) {
    this->settings->scope.horizontal.recordLength = this->recordLengthComboBox->itemData(index).toUInt();
    if(!this->suppressSignals)
        emit recordLengthChanged(index);
}

/// \brief Called when the format combo box changes its value.
/// \param index The index of the combo box item.
void HorizontalDock::formatSelected(int index) {
    this->settings->scope.horizontal.format = (GraphFormat) index;
    if(!this->suppressSignals)
        emit formatChanged(this->settings->scope.horizontal.format);
}


////////////////////////////////////////////////////////////////////////////////
// class TriggerDock
/// \brief Initializes the trigger settings docking window.
/// \param settings The target settings object.
/// \param specialTriggers The names of the special trigger sources.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
TriggerDock::TriggerDock(OpenHantekSettings *settings, QWidget *parent, Qt::WindowFlags flags)
: QDockWidget(tr("Trigger"), parent, flags) {
    this->settings = settings;

    // Initialize elements
    this->modeLabel = new QLabel(tr("Mode"));
    this->modeComboBox = new QComboBox();

    this->slopeLabel = new QLabel(tr("Slope"));
    this->slopeComboBox = new QComboBox();

    this->sourceLabel = new QLabel(tr("Source"));
    this->sourceComboBox = new QComboBox();

    this->dockLayout = new QGridLayout();
    this->dockLayout->setColumnMinimumWidth(0, 64);
    this->dockLayout->setColumnStretch(1, 1);
    this->dockLayout->addWidget(this->modeLabel, 0, 0);
    this->dockLayout->addWidget(this->modeComboBox, 0, 1);
    this->dockLayout->addWidget(this->sourceLabel, 1, 0);
    this->dockLayout->addWidget(this->sourceComboBox, 1, 1);
    this->dockLayout->addWidget(this->slopeLabel, 2, 0);
    this->dockLayout->addWidget(this->slopeComboBox, 2, 1);

    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    this->dockWidget = new QWidget();
    this->dockWidget->setLayout(this->dockLayout);
    this->setWidget(this->dockWidget);

    // Connect signals and slots
    connect(this->modeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &TriggerDock::modeSelected);
    connect(this->slopeComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &TriggerDock::slopeSelected);
    connect(this->sourceComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &TriggerDock::sourceSelected);

    // Set values
    this->setMode(this->settings->scope.trigger.mode);
    this->setSlope(this->settings->scope.trigger.slope);
    this->setSource(this->settings->scope.trigger.special, this->settings->scope.trigger.source);
}

/// \brief Cleans up everything.
TriggerDock::~TriggerDock() {
}

/// \brief Don't close the dock, just hide it 
/// \param event The close event that should be handled.
void TriggerDock::closeEvent(QCloseEvent *event) {
    this->hide();

    event->accept();
}

/// \brief Changes the trigger mode if the new mode is supported.
/// \param mode The trigger mode.
/// \return Index of mode-value, -1 on error.
void TriggerDock::setChannels(const std::vector<std::string> &specialTriggers) {
    // Initialize lists for comboboxes
    for(unsigned channel = 0; channel < this->settings->device->getChannelCount(); ++channel)
        this->sourceStandardStrings << tr("CH%1").arg(channel + 1);
    for(const std::string& s: specialTriggers)
        this->sourceSpecialStrings << QString::fromStdString(s);

    this->modeComboBox->clear();
    for(int mode = (int)DSO::TriggerMode::AUTO; mode < (int)DSO::TriggerMode::COUNT; ++mode)
        this->modeComboBox->addItem(DsoStrings::triggerModeString((DSO::TriggerMode) mode));

    this->slopeComboBox->clear();
    for(int slope = (int)DSO::Slope::POSITIVE; slope < (int)DSO::Slope::COUNT; ++slope)
        this->slopeComboBox->addItem(DsoStrings::slopeString((DSO::Slope) slope));

    this->sourceComboBox->clear();
    this->sourceComboBox->addItems(this->sourceStandardStrings);
    this->sourceComboBox->addItems(this->sourceSpecialStrings);
}

/// \brief Changes the trigger mode if the new mode is supported.
/// \param mode The trigger mode.
/// \return Index of mode-value, -1 on error.
int TriggerDock::setMode(DSO::TriggerMode mode) {
    if(mode >= DSO::TriggerMode::AUTO && mode <=DSO::TriggerMode::SINGLE) {
        this->modeComboBox->setCurrentIndex((int)mode);
        return (int)mode;
    }

    return -1;
}

/// \brief Changes the trigger slope if the new slope is supported.
/// \param slope The trigger slope.
/// \return Index of slope-value, -1 on error.
int TriggerDock::setSlope(DSO::Slope slope) {
    if(slope >= DSO::Slope::POSITIVE && slope <= DSO::Slope::NEGATIVE) {
        this->slopeComboBox->setCurrentIndex((int)slope);
        return (int)slope;
    }

    return -1;
}

/// \brief Changes the trigger source if the new source is supported.
/// \param special true for a special channel (EXT, ...) as trigger source.
/// \param id The number of the channel, that should be used as trigger.
/// \return Index of source item, -1 on error.
int TriggerDock::setSource(bool special, unsigned int id) {
    if((!special && id >= (unsigned int) this->sourceStandardStrings.count()) || (special && id >= (unsigned int) this->sourceSpecialStrings.count()))
        return -1;

    int index = id;
    if(special)
        index += this->sourceStandardStrings.count();
    this->sourceComboBox->setCurrentIndex(index);

    return index;
}

/// \brief Called when the mode combo box changes it's value.
/// \param index The index of the combo box item.
void TriggerDock::modeSelected(int index) {
    this->settings->scope.trigger.mode = (DSO::TriggerMode) index;
    emit modeChanged(this->settings->scope.trigger.mode);
}

/// \brief Called when the slope combo box changes it's value.
/// \param index The index of the combo box item.
void TriggerDock::slopeSelected(int index) {
    this->settings->scope.trigger.slope = (DSO::Slope) index;
    emit slopeChanged(this->settings->scope.trigger.slope);
}

/// \brief Called when the source combo box changes it's value.
/// \param index The index of the combo box item.
void TriggerDock::sourceSelected(int index) {
    unsigned int id = index;
    bool special = false;

    if(id >= (unsigned int) this->sourceStandardStrings.count()) {
        id -= this->sourceStandardStrings.count();
        special = true;
    }

    this->settings->scope.trigger.source = id;
    this->settings->scope.trigger.special = special;
    emit sourceChanged(special, id);
}


////////////////////////////////////////////////////////////////////////////////
// class SpectrumDock
/// \brief Initializes the spectrum view docking window.
/// \param settings The target settings object.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
SpectrumDock::SpectrumDock(OpenHantekSettings *settings, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(tr("Spectrum"), parent, flags) {
    this->settings = settings;

    // Initialize lists for comboboxes
    this->magnitudeSteps                <<  1e0 <<  2e0 <<  3e0 <<  6e0
            <<  1e1 <<  2e1 <<  3e1 <<  6e1 <<  1e2 <<  2e2 <<  3e2 <<  6e2; ///< Magnitude steps in dB/div
    for(QList<double>::iterator magnitude = this->magnitudeSteps.begin(); magnitude != this->magnitudeSteps.end(); ++magnitude)
        this->magnitudeStrings << UnitToString::valueToString(*magnitude, UnitToString::UNIT_DECIBEL, 0);

    // Initialize elements
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        this->magnitudeComboBox.append(new QComboBox());
        this->magnitudeComboBox[channel]->addItems(this->magnitudeStrings);

        this->usedCheckBox.append(new QCheckBox(QString::fromStdString(this->settings->scope.voltage[channel].name)));
    }

    this->dockLayout = new QGridLayout();
    this->dockLayout->setColumnMinimumWidth(0, 64);
    this->dockLayout->setColumnStretch(1, 1);
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        this->dockLayout->addWidget(this->usedCheckBox[channel], channel, 0);
        this->dockLayout->addWidget(this->magnitudeComboBox[channel], channel, 1);
    }

    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    this->dockWidget = new QWidget();
    this->dockWidget->setLayout(this->dockLayout);
    this->setWidget(this->dockWidget);

    // Connect signals and slots
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        connect(this->magnitudeComboBox[channel], static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &SpectrumDock::magnitudeSelected);
        connect(this->usedCheckBox[channel], &QCheckBox::toggled,
                this, &SpectrumDock::usedSwitched);
    }

    // Set values
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        this->setMagnitude(channel, this->settings->scope.spectrum[channel].magnitude);
        this->setUsed(channel, this->settings->scope.spectrum[channel].used);
    }
}

/// \brief Cleans up everything.
SpectrumDock::~SpectrumDock() {
}

/// \brief Don't close the dock, just hide it 
/// \param event The close event that should be handled.
void SpectrumDock::closeEvent(QCloseEvent *event) {
    this->hide();

    event->accept();
}

/// \brief Sets the magnitude for a channel.
/// \param channel The channel, whose magnitude should be set.
/// \param magnitude The magnitude in dB.
/// \return Index of magnitude-value, -1 on error.
int SpectrumDock::setMagnitude(unsigned channel, double magnitude) {
    if(channel < 0 || channel >= this->settings->scope.voltage.size())
        return -1;

    int index = this->magnitudeSteps.indexOf(magnitude);
    if(index != -1)
        this->magnitudeComboBox[channel]->setCurrentIndex(index);

    return index;
}

/// \brief Enables/disables a channel.
/// \param channel The channel, that should be enabled/disabled.
/// \param used True if the channel should be enabled, false otherwise.
/// \return Index of channel, -1 on error.
int SpectrumDock::setUsed(unsigned channel, bool used) {
    if(channel >= 0 && channel < this->settings->scope.voltage.size()) {
        this->usedCheckBox[channel]->setChecked(used);
        return channel;
    }

    return -1;
}

/// \brief Called when the source combo box changes it's value.
/// \param index The index of the combo box item.
void SpectrumDock::magnitudeSelected(int index) {
    unsigned channel;

    // Which combobox was it?
    for(channel = 0; channel < this->settings->scope.voltage.size(); ++channel)
        if(this->sender() == this->magnitudeComboBox[channel])
            break;

    // Send signal if it was one of the comboboxes
    if(channel < this->settings->scope.voltage.size()) {
        this->settings->scope.spectrum[channel].magnitude = this->magnitudeSteps.at(index);
        emit magnitudeChanged(channel, this->settings->scope.spectrum[channel].magnitude);
    }
}

/// \brief Called when the used checkbox is switched.
/// \param checked The check-state of the checkbox.
void SpectrumDock::usedSwitched(bool checked) {
    unsigned channel;

    // Which checkbox was it?
    for(channel = 0; channel < this->settings->scope.voltage.size(); ++channel)
        if(this->sender() == this->usedCheckBox[channel])
            break;

    // Send signal if it was one of the checkboxes
    if(channel < this->settings->scope.voltage.size()) {
        this->settings->scope.spectrum[channel].used = checked;
        emit usedChanged(channel, checked);
    }
}


////////////////////////////////////////////////////////////////////////////////
// class VoltageDock
/// \brief Initializes the vertical axis docking window.
/// \param settings The target settings object.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
VoltageDock::VoltageDock(OpenHantekSettings *settings, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(tr("Voltage"), parent, flags) {
    this->settings = settings;

    // Initialize lists for comboboxes
    for(int coupling = 0; coupling < (int)DSO::Coupling::COUNT; ++coupling)
        this->couplingStrings.append(DsoStrings::couplingString((DSO::Coupling) coupling));

    for(int mode = 0; mode < (int)DSOAnalyser::MathMode::COUNT; ++mode)
        this->modeStrings.append(DsoStrings::mathModeString((DSOAnalyser::MathMode) mode));

    this->gainSteps             << 1e-2 << 2e-2 << 5e-2 << 1e-1 << 2e-1 << 5e-1
            <<  1e0 <<  2e0 <<  5e0;          ///< Voltage steps in V/div
    for(QList<double>::iterator gain = this->gainSteps.begin(); gain != this->gainSteps.end(); ++gain)
        this->gainStrings << UnitToString::valueToString(*gain, UnitToString::UNIT_VOLTS, 0);

    // Initialize elements
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        this->miscComboBox.append(new QComboBox());
        if(channel < this->settings->device->getChannelCount())
            this->miscComboBox[channel]->addItems(this->couplingStrings);
        else
            this->miscComboBox[channel]->addItems(this->modeStrings);

        this->gainComboBox.append(new QComboBox());
        this->gainComboBox[channel]->addItems(this->gainStrings);

        this->usedCheckBox.append(new QCheckBox(QString::fromStdString(this->settings->scope.voltage[channel].name)));
    }

    this->dockLayout = new QGridLayout();
    this->dockLayout->setColumnMinimumWidth(0, 64);
    this->dockLayout->setColumnStretch(1, 1);
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        this->dockLayout->addWidget(this->usedCheckBox[channel], channel * 2, 0);
        this->dockLayout->addWidget(this->gainComboBox[channel], channel * 2, 1);
        this->dockLayout->addWidget(this->miscComboBox[channel], channel * 2 + 1, 1);
    }

    this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    this->dockWidget = new QWidget();
    this->dockWidget->setLayout(this->dockLayout);
    this->setWidget(this->dockWidget);

    // Connect signals and slots
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        connect(this->gainComboBox[channel], static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &VoltageDock::gainSelected);
        connect(this->miscComboBox[channel], static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &VoltageDock::miscSelected);
        connect(this->usedCheckBox[channel], &QCheckBox::toggled,
                this, &VoltageDock::usedSwitched);
    }

    // Set values
    for(unsigned channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {
        if(channel < this->settings->device->getChannelCount())
            this->setCoupling(channel, this->settings->scope.voltage[channel].coupling);
        else
            this->setMode(this->settings->scope.voltage[channel].mathmode);
        this->setGain(channel, this->settings->scope.voltage[channel].gain);
        this->setUsed(channel, this->settings->scope.voltage[channel].used);
    }
}

/// \brief Cleans up everything.
VoltageDock::~VoltageDock() {
}

/// \brief Don't close the dock, just hide it 
/// \param event The close event that should be handled.
void VoltageDock::closeEvent(QCloseEvent *event) {
    this->hide();

    event->accept();
}

/// \brief Sets the coupling for a channel.
/// \param channel The channel, whose coupling should be set.
/// \param coupling The coupling-mode.
/// \return Index of coupling-mode, -1 on error.
int VoltageDock::setCoupling(unsigned channel, DSO::Coupling coupling) {
    if(coupling < DSO::Coupling::AC || coupling > DSO::Coupling::GND)
        return -1;
    if(channel < 0 || channel >= this->settings->device->getChannelCount())
        return -1;

    this->miscComboBox[channel]->setCurrentIndex((int)coupling);
    return (int)coupling;
}

/// \brief Sets the gain for a channel.
/// \param channel The channel, whose gain should be set.
/// \param gain The gain in volts.
/// \return Index of gain-value, -1 on error.
int VoltageDock::setGain(unsigned channel, double gain) {
    if(channel < 0 || channel >= this->settings->scope.voltage.size())
        return -1;

    int index = this->gainSteps.indexOf(gain);
    if(index != -1)
        this->gainComboBox[channel]->setCurrentIndex(index);

    return index;
}

/// \brief Sets the mode for the math channel.
/// \param mode The math-mode.
/// \return Index of math-mode, -1 on error.
int VoltageDock::setMode(DSOAnalyser::MathMode mode) {
    if((int)mode >= 0 && mode < DSOAnalyser::MathMode::COUNT) {
        this->miscComboBox[this->settings->device->getChannelCount()]->setCurrentIndex((int)mode);
        return (int)mode;
    }

    return -1;
}

/// \brief Enables/disables a channel.
/// \param channel The channel, that should be enabled/disabled.
/// \param used True if the channel should be enabled, false otherwise.
/// \return Index of channel, -1 on error.
int VoltageDock::setUsed(unsigned channel, bool used) {
    if(channel >= 0 && channel < this->settings->scope.voltage.size()) {
        this->usedCheckBox[channel]->setChecked(used);
        return channel;
    }

    return -1;
}

/// \brief Called when the gain combo box changes it's value.
/// \param index The index of the combo box item.
void VoltageDock::gainSelected(int index) {
    unsigned channel;

    // Which combobox was it?
    for(channel = 0; channel < this->settings->scope.voltage.size(); ++channel)
        if(this->sender() == this->gainComboBox[channel])
            break;

    // Send signal if it was one of the comboboxes
    if(channel < this->settings->scope.voltage.size()) {
        this->settings->scope.voltage[channel].gain = this->gainSteps.at(index);

        emit gainChanged(channel, this->settings->scope.voltage[channel].gain);
    }
}

/// \brief Called when the misc combo box changes it's value.
/// \param index The index of the combo box item.
void VoltageDock::miscSelected(int index) {
    unsigned channel;

    // Which combobox was it?
    for(channel = 0; channel < this->settings->scope.voltage.size(); ++channel)
        if(this->sender() == this->miscComboBox[channel])
            break;

    // Send signal if it was one of the comboboxes
    if(channel < this->settings->scope.voltage.size()) {
        if(channel < this->settings->device->getChannelCount()) {
            this->settings->scope.voltage[channel].coupling = (DSO::Coupling) index;
            emit couplingChanged(channel, this->settings->scope.voltage[channel].coupling);
        } else {
            this->settings->scope.voltage[channel].mathmode = (DSOAnalyser::MathMode) index;
            emit modeChanged((DSOAnalyser::MathMode) this->settings->scope.voltage[channel].mathmode);
        }
    }
}

/// \brief Called when the used checkbox is switched.
/// \param checked The check-state of the checkbox.
void VoltageDock::usedSwitched(bool checked) {
    unsigned channel;

    // Which checkbox was it?
    for(channel = 0; channel < this->settings->scope.voltage.size(); ++channel)
        if(this->sender() == this->usedCheckBox[channel])
            break;
    
    // Send signal if it was one of the checkboxes
    if(channel < this->settings->scope.voltage.size()) {
        this->settings->scope.voltage[channel].used = checked;
        emit usedChanged(channel, checked);
    }
}
