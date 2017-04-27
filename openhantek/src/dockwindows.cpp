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
#include <QDebug>
#include <QStringList>

#include "dockwindows.h"

#include "settings.h"
#include "glgenerator.h"
#include "deviceBaseSpecifications.h"
#include "currentdevice.h"
#include "dataAnalyzerSettings.h"
#include "sispinbox.h"
#include "helper.h"
//#include "hantek/types.h"
#include "requests.h"

#define HANTEK_CHANNELS 2
////////////////////////////////////////////////////////////////////////////////
// class HorizontalDock
/// \brief Initializes the horizontal axis docking window.
/// \param settings The target settings object.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
HorizontalDock::HorizontalDock(DsoSettings *settings, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(tr("Horizontal"), parent, flags) {
	this->settings = settings;
	
	// Initialize elements

	this->samplerateLabel = new QLabel(tr("Samplerate"));
	this->samplerateSiSpinBox = new SiSpinBox(Helper::UNIT_SAMPLES);
    this->samplerateSiSpinBox->setMinimum(100);
    this->samplerateSiSpinBox->setMaximum(1e8);
    this->samplerateSiSpinBox->setUnitPostfix("/s");

    this->samplerateComboBox = new QComboBox(this);
    QList<QString> sampleRates;

    sampleRates <<QString("48 MHz") << QString("30 MHz") << QString("24 MHz") << QString("16 MHz") << QString("8 MHz") << QString("4 MHz")
               << QString("1 MHz");
    this->samplerateComboBox->addItems(sampleRates);

	QList<double> timebaseSteps;
    timebaseSteps << 1.0 << 2.0 << 5.0 << 10.0;
	
	this->timebaseLabel = new QLabel(tr("Timebase"));
	this->timebaseSiSpinBox = new SiSpinBox(Helper::UNIT_SECONDS);
	this->timebaseSiSpinBox->setSteps(timebaseSteps);
    this->timebaseSiSpinBox->setMinimum(2e-6);
//    this->timebaseSiSpinBox->setMaximum(100e3);
    this->timebaseSiSpinBox->setMaximum(0.2);
    this->timebaseSiSpinBox->setValue(1e-4);
	
	this->frequencybaseLabel = new QLabel(tr("Frequencybase"));
	this->frequencybaseSiSpinBox = new SiSpinBox(Helper::UNIT_HERTZ);
	this->frequencybaseSiSpinBox->setMinimum(1.0);
    this->frequencybaseSiSpinBox->setMaximum(50e6);
	
	this->recordLengthLabel = new QLabel(tr("Record length"));
    this->recordLengthComboBox = new QComboBox();

    QList<QString> recordLength;
    recordLength << "1016" << "130048" << "5223264" << "1047552";
    this->recordLengthComboBox->addItems(recordLength);

	this->formatLabel = new QLabel(tr("Format"));
	this->formatComboBox = new QComboBox();
    for(int format = DSO::GraphFormat::GRAPHFORMAT_TY; format < DSO::GraphFormat::GRAPHFORMAT_COUNT; ++format)
        this->formatComboBox->addItem(Dso::graphFormatString((DSO::GraphFormat) format));
	
	this->dockLayout = new QGridLayout();
	this->dockLayout->setColumnMinimumWidth(0, 64);
	this->dockLayout->setColumnStretch(1, 1);
	this->dockLayout->addWidget(this->samplerateLabel, 0, 0);
//	this->dockLayout->addWidget(this->samplerateSiSpinBox, 0, 1);
    this->dockLayout->addWidget(this->samplerateComboBox, 0, 1);
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
	connect(this->samplerateSiSpinBox, SIGNAL(valueChanged(double)), this, SLOT(samplerateSelected(double)));
	connect(this->timebaseSiSpinBox, SIGNAL(valueChanged(double)), this, SLOT(timebaseSelected(double)));
	connect(this->frequencybaseSiSpinBox, SIGNAL(valueChanged(double)), this, SLOT(frequencybaseSelected(double)));
	connect(this->recordLengthComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(recordLengthSelected(int)));
	connect(this->formatComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(formatSelected(int)));
	
	// Set values
	this->setSamplerate(this->settings->scope.horizontal.samplerate);
	this->setTimebase(this->settings->scope.horizontal.timebase);
	this->setFrequencybase(this->settings->scope.horizontal.frequencybase);
	this->setRecordLength(this->settings->scope.horizontal.recordLength);
	this->setFormat(this->settings->scope.horizontal.format);
}

/// \brief Cleans up everything.
HorizontalDock::~HorizontalDock() {
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
    qDebug() << "set timebase to " << timebase << endl;
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
int HorizontalDock::setFormat(DSO::GraphFormat format) {
    if(format >= DSO::GRAPHFORMAT_TY && format <= DSO::GRAPHFORMAT_XY) {
		this->suppressSignals = true;
		this->formatComboBox->setCurrentIndex(format);
		this->suppressSignals = false;
		return format;
	}
	
	return -1;
}

/// \brief Updates the available record lengths in the combo box.
/// \param recordLengths The available record lengths for the combo box.
void HorizontalDock::availableRecordLengthsChanged(const QList<unsigned int> &recordLengths) {
	/// \todo Empty lists should be interpreted as scope supporting continuous record length values.

    this->recordLengthComboBox->blockSignals(true); // Avoid messing up the settings
	this->recordLengthComboBox->setUpdatesEnabled(false);
	
	// Update existing elements to avoid unnecessary index updates
	int index = 0;
	for(; index < recordLengths.size(); ++index) {
		unsigned int recordLengthItem = recordLengths[index];
		if(index < this->recordLengthComboBox->count()) {
			this->recordLengthComboBox->setItemData(index, recordLengthItem);
			this->recordLengthComboBox->setItemText(index, recordLengthItem == UINT_MAX ? tr("Roll") : Helper::valueToString(recordLengthItem, Helper::UNIT_SAMPLES, 3));
		}
		else {
			this->recordLengthComboBox->addItem(recordLengthItem == UINT_MAX ? tr("Roll") : Helper::valueToString(recordLengthItem, Helper::UNIT_SAMPLES, 3), (uint) recordLengthItem);
		}
	}
	// Remove extra elements
	for(int extraIndex = this->recordLengthComboBox->count() - 1; extraIndex > index; --extraIndex) {
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

void HorizontalDock::hard_event(int type, int value)
{
	switch (type) {
	case PANEL_ENC_H_TIMEBASE:
        timebaseSiSpinBox->stepBy(-value);
		break;
	}
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
    qDebug() << "timebase selected: " << timebase << endl;
    if (m_device == nullptr)
        return;
    m_device->setTimebase(timebase);
    /* update the downsampling rate in the scope settings */
    this->settings->scope.horizontal.downsampling = m_device->getDownsamplerRateFromTimebase(timebase);
    const std::vector<DSO::dsoAvailableSamplingRate> &availableSamplingRates= m_device->getAvailableSamplingrates();

    double timeValue = this->timebaseSiSpinBox->value();
    int index= -1;

    qDebug() << "no of timbase values: " << availableSamplingRates.size() << endl;
    for (int i=0; i< availableSamplingRates.size(); i++) {
//        qDebug() << "index: " << i << " timebase: " <<  availableSamplingRates[i].timeBase << "timebase to be searched: "<< timeValue << endl;
        if (fabs(timeValue-availableSamplingRates[i].timeBase) < 1e-9) {
            index = i;
            break;
        }
    }
    int recordLengthID;
    double samplingrateValue;
    if (index == -1) {
        qDebug() << "timeValue " << timeValue << " not found" << endl;
        return;
    }
    else {
        samplingrateValue = availableSamplingRates[index].samplingrateValue;
        m_device->setSamplerate(samplingrateValue);
        recordLengthID = availableSamplingRates[index].recordLengthID;
        m_device->setRecordLengthByID((DSO::HWRecordLengthID)recordLengthID);
        qDebug() << "HorizontalDock::timebaseSelected samplerate: " << samplingrateValue << " record length index: " << recordLengthID << endl;
        m_device->setTimebase(timebase);
    }
    for (int i=0;i<samplingrateStrings.size();++i)
        qDebug() << samplingrateStrings[i] << endl;

    QString sampleRateString = Helper::valueToString(samplingrateValue,Helper::UNIT_HERTZ,0);
    index = -1;
    for (int i=0; i<samplingrateStrings.count();++i)
        if (!QString::compare(sampleRateString,samplingrateStrings[i])) {
            qDebug() << "samplerate string: " << sampleRateString << "index: " << i << endl;
            index = i;
        }
   samplerateComboBox->setCurrentIndex(index);

/* find the index for the record length corresponding to this time base */
    index = -1;
    const std::vector<DSO::HWRecordLengthID> &recordLength= m_device->getRecordLength();
    for (int i=0; i<recordLength.size();++i) {
        qDebug() << "index: " << i << " record length " <<  recordLength[i]  << endl;
        if (recordLength[i] == recordLengthID) {
            index = i;
        }
    }
    if (index == -1) {
        qDebug() << "record length " << recordLengthID << " not found" << endl;
        return;
    }
    else {
        recordLengthComboBox->blockSignals(false);
        recordLengthComboBox->setCurrentIndex(index);
        qDebug() << "Setting record length index to " << index << endl;
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
    this->settings->scope.horizontal.format = (DSO::GraphFormat) index;
	if(!this->suppressSignals)
		emit formatChanged(this->settings->scope.horizontal.format);
}

void HorizontalDock::updateSpecifications() {
    CurrentDevice *sender = (CurrentDevice *) QObject::sender();
//    qDebug() << "HorizontalDock::updateSpecifications: update sampling rate and record length specs "  << endl;
    this->m_device = sender->getCurrentDevice();
    const std::vector<DSO::dsoAvailableSamplingRate> &availableSamplingRates= m_device->getAvailableSamplingrates();
//    qDebug() << "no of sampling rate values: " << availableSamplingRates.size();
    this->samplingrateStrings.clear();
    double timebase = timebaseSiSpinBox->value();
    m_device->setTimebase(timebase);

    double samplerate = m_device->getSamplingrateFromTimebase(timebase);

    qDebug() << "current timebase value " << timebase << " samplerate: " << samplerate << endl;
    m_device->setSamplerate(samplerate);
    int index=-1;
    double last = -1;
    int srIndex = -1;

    for (unsigned int sr=0; sr<availableSamplingRates.size(); ++sr) {
        if (availableSamplingRates[sr].samplingrateValue != last) {
            samplingrateStrings.append(Helper::valueToString(availableSamplingRates[sr].samplingrateValue,Helper::UNIT_HERTZ,0));
            srIndex++;
            last = availableSamplingRates[sr].samplingrateValue;
        }
        if ((index == -1) && (fabs(availableSamplingRates[sr].samplingrateValue-1e6) < 1e-9)) {
            index = srIndex;
        }

    }
    qDebug() << "srIndex: " << index << endl;

    samplerateComboBox->blockSignals(true);
    samplerateComboBox->clear();
    samplerateComboBox->addItems(samplingrateStrings);
    samplerateComboBox->setCurrentIndex(index);
    samplerateComboBox->blockSignals(false);

    recordLengthStrings.clear();
    const std::vector<DSO::HWRecordLengthID> &recordLength= m_device->getRecordLength();
    recordLengthStrings.clear();
    for (int rl=0; rl<recordLength.size();++rl) {
        recordLengthStrings.append(Helper::valueToString(recordLength[rl],Helper::UNIT_SAMPLES,0));
        if (recordLength[rl] == DSO::RECORDLENGTH_10KB)
            index = rl;
    }
    recordLengthComboBox->blockSignals(true);
    recordLengthComboBox->clear();
    recordLengthComboBox->addItems(recordLengthStrings);
    recordLengthComboBox->setCurrentIndex(index);
    recordLengthComboBox->blockSignals(false);

}

////////////////////////////////////////////////////////////////////////////////
// class TriggerDock
/// \brief Initializes the trigger settings docking window.
/// \param settings The target settings object.
/// \param specialTriggers The names of the special trigger sources.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
TriggerDock::TriggerDock(DsoSettings *settings, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(tr("Trigger"), parent, flags) {
	this->settings = settings;
	
	// Initialize lists for comboboxes
	for(unsigned int channel = 0; channel < this->settings->scope.physicalChannels; ++channel)
		this->sourceStandardStrings << tr("CH%1").arg(channel + 1);
	
	// Initialize elements
	this->modeLabel = new QLabel(tr("Mode"));
    this->modeComboBox = new QComboBox();

    for(int mode = (int)DSO::TriggerMode::TRIGGERMODE_AUTO; mode < (int)DSO::TriggerMode::TRIGGERMODE_SINGLE+1; mode++) {
        this->modeComboBox->addItem(Dso::triggerModeString((DSO::TriggerMode) mode));
//        qDebug() << "Trigger Combobox: mode: " << mode << "string: " << Dso::triggerModeString((DSO::TriggerMode) mode) << endl;
    }
	this->slopeLabel = new QLabel(tr("Slope"));
	this->slopeComboBox = new QComboBox();
    for(int slope = (int)DSO::Slope::SLOPE_POSITIVE; slope < DSO::Slope::SLOPE_COUNT; ++slope)
        this->slopeComboBox->addItem(Dso::slopeString((DSO::Slope) slope));
	this->sourceLabel = new QLabel(tr("Source"));
	this->sourceComboBox = new QComboBox();
	this->sourceComboBox->addItems(this->sourceStandardStrings);
	this->sourceComboBox->addItems(this->sourceSpecialStrings);

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
	connect(this->modeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(modeSelected(int)));
	connect(this->slopeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(slopeSelected(int)));
	connect(this->sourceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceSelected(int)));
	
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
int TriggerDock::setMode(DSO::TriggerMode mode) {
    if(mode >= DSO::TriggerMode::TRIGGERMODE_AUTO  && mode <= DSO::TriggerMode::TRIGGERMODE_SINGLE) {
        this->modeComboBox->setCurrentIndex((int)mode);
        return (int)mode;
	}
	
	return -1;
}

/// \brief Changes the trigger slope if the new slope is supported.
/// \param slope The trigger slope.
/// \return Index of slope-value, -1 on error.
int TriggerDock::setSlope(DSO::Slope slope) {
    if(slope >= DSO::Slope::SLOPE_POSITIVE && slope <= DSO::Slope::SLOPE_NEGATIVE) {
		this->slopeComboBox->setCurrentIndex(slope);
		return slope;
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

void TriggerDock::hard_event(int type, int value)
{
    Q_UNUSED(value);
	switch (type) {
	case PANEL_SW_R_MODE:
		modeComboBox->setCurrentIndex((modeComboBox->currentIndex() + 1) % modeComboBox->count());
		break;
	case PANEL_SW_T_EDGE:
		slopeComboBox->setCurrentIndex((slopeComboBox->currentIndex() + 1) % slopeComboBox->count());
		break;
	case PANEL_SW_T_SOURCE:
		sourceComboBox->setCurrentIndex((sourceComboBox->currentIndex() + 1) % HANTEK_CHANNELS);
		break;
	}
}


////////////////////////////////////////////////////////////////////////////////
// class SpectrumDock
/// \brief Initializes the spectrum view docking window.
/// \param settings The target settings object.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
SpectrumDock::SpectrumDock(DsoSettings *settings, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(tr("Spectrum"), parent, flags) {
	this->settings = settings;
	
	// Initialize lists for comboboxes
	this->magnitudeSteps                <<  1e0 <<  2e0 <<  3e0 <<  6e0
			<<  1e1 <<  2e1 <<  3e1 <<  6e1 <<  1e2 <<  2e2 <<  3e2 <<  6e2; ///< Magnitude steps in dB/div
	for(QList<double>::iterator magnitude = this->magnitudeSteps.begin(); magnitude != this->magnitudeSteps.end(); ++magnitude)
		this->magnitudeStrings << Helper::valueToString(*magnitude, Helper::UNIT_DECIBEL, 0);
	
	// Initialize elements
	for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
		this->magnitudeComboBox.append(new QComboBox());
		this->magnitudeComboBox[channel]->addItems(this->magnitudeStrings);
        this->usedCheckBox.append(new QCheckBox(QString::fromStdString(this->settings->scope.voltage[channel].name)));
//        this->usedCheckBox.append(new QCheckBox(this->settings->scope.voltage[channel].name));
	}
	
	this->dockLayout = new QGridLayout();
	this->dockLayout->setColumnMinimumWidth(0, 64);
	this->dockLayout->setColumnStretch(1, 1);
	for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
		this->dockLayout->addWidget(this->usedCheckBox[channel], channel, 0);
		this->dockLayout->addWidget(this->magnitudeComboBox[channel], channel, 1);
	}

	this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	
	this->dockWidget = new QWidget();
	this->dockWidget->setLayout(this->dockLayout);
	this->setWidget(this->dockWidget);
	
	// Connect signals and slots
	for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
		connect(this->magnitudeComboBox[channel], SIGNAL(currentIndexChanged(int)), this, SLOT(magnitudeSelected(int)));
		connect(this->usedCheckBox[channel], SIGNAL(toggled(bool)), this, SLOT(usedSwitched(bool)));
	}
	
	// Set values
	for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
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
int SpectrumDock::setMagnitude(int channel, double magnitude) {
	if(channel < 0 || channel >= this->settings->scope.voltage.count())
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
int SpectrumDock::setUsed(int channel, bool used) {
	if(channel >= 0 && channel < this->settings->scope.voltage.count()) {
		this->usedCheckBox[channel]->setChecked(used);
		return channel;
	}
	
	return -1;
}

/// \brief Called when the source combo box changes it's value.
/// \param index The index of the combo box item.
void SpectrumDock::magnitudeSelected(int index) {
	int channel;
	
	// Which combobox was it?
	for(channel = 0; channel < this->settings->scope.voltage.count(); ++channel)
		if(this->sender() == this->magnitudeComboBox[channel])
			break;
	
	// Send signal if it was one of the comboboxes
	if(channel < this->settings->scope.voltage.count()) {
		this->settings->scope.spectrum[channel].magnitude = this->magnitudeSteps.at(index);
		emit magnitudeChanged(channel, this->settings->scope.spectrum[channel].magnitude);
	}
}

/// \brief Called when the used checkbox is switched.
/// \param checked The check-state of the checkbox.
void SpectrumDock::usedSwitched(bool checked) {
	int channel;
	
	// Which checkbox was it?
	for(channel = 0; channel < this->settings->scope.voltage.count(); ++channel)
		if(this->sender() == this->usedCheckBox[channel])
			break;
	
	// Send signal if it was one of the checkboxes
	if(channel < this->settings->scope.voltage.count()) {
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
VoltageDock::VoltageDock(DsoSettings *settings, QWidget *parent, Qt::WindowFlags flags) : QDockWidget(tr("Voltage"), parent, flags) {
	this->settings = settings;

    // Initialize lists for comboboxes
    /*
    for(int coupling = Dso::COUPLING_AC; coupling < Dso::COUPLING_COUNT; ++coupling)
        this->couplingStrings.append(Dso::couplingString((Dso::Coupling) coupling));
    */
    /*
     * Hantek 6022 has only DC coupling
     * This should be picked oup from specifications
     */
    this->couplingStrings.append(Dso::couplingString((DSO::Coupling) DSO::Coupling::COUPLING_DC));

    for(int mode = (int)DSOAnalyzer::MathMode::ADD_CH1_CH2; mode < (int)DSOAnalyzer::MathMode::MATHMODE_COUNT; ++mode) {
        this->modeStrings.append(Dso::mathModeString((DSOAnalyzer::MathMode) mode));
        qDebug() << "math mode: " << Dso::mathModeString((DSOAnalyzer::MathMode) mode) << endl;
}
    this->gainSteps  << 2e-2 << 5e-2 << 1e-1 << 2e-1 << 5e-1
            <<  1e0 <<  2e0 <<  5e0;          ///< Voltage steps in V/div

    for(QList<double>::iterator gain = this->gainSteps.begin(); gain != this->gainSteps.end(); ++gain)
        this->gainStrings << Helper::valueToString(*gain, Helper::UNIT_VOLTS, 0);

    // Initialize elements
    for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
        this->miscComboBox.append(new QComboBox());
        if(channel < (int) this->settings->scope.physicalChannels)
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
	for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
		this->dockLayout->addWidget(this->usedCheckBox[channel], channel * 2, 0);
		this->dockLayout->addWidget(this->gainComboBox[channel], channel * 2, 1);
		this->dockLayout->addWidget(this->miscComboBox[channel], channel * 2 + 1, 1);
	}

	this->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	
	this->dockWidget = new QWidget();
	this->dockWidget->setLayout(this->dockLayout);
	this->setWidget(this->dockWidget);
	
	// Connect signals and slots
	for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
		connect(this->gainComboBox[channel], SIGNAL(currentIndexChanged(int)), this, SLOT(gainSelected(int)));
		connect(this->miscComboBox[channel], SIGNAL(currentIndexChanged(int)), this, SLOT(miscSelected(int)));
		connect(this->usedCheckBox[channel], SIGNAL(toggled(bool)), this, SLOT(usedSwitched(bool)));
	}
	
	// Set values
	for(int channel = 0; channel < this->settings->scope.voltage.count(); ++channel) {
		if(channel < (int) this->settings->scope.physicalChannels)
            this->setCoupling(channel, (DSO::Coupling) this->settings->scope.voltage[channel].misc);
		else
            this->setMode((DSOAnalyzer::MathMode) this->settings->scope.voltage[channel].misc);
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

void VoltageDock::setDevice(std::shared_ptr<DSO::DeviceBase> device) {
    this->m_device = device;
}

/// \brief Sets the coupling for a channel.
/// \param channel The channel, whose coupling should be set.
/// \param coupling The coupling-mode.
/// \return Index of coupling-mode, -1 on error.
int VoltageDock::setCoupling(int channel, DSO::Coupling coupling) {
    coupling = DSO::Coupling::COUPLING_DC;
    /*
	if(coupling < Dso::COUPLING_AC || coupling > Dso::COUPLING_GND)
		return -1;
	if(channel < 0 || channel >= (int) this->settings->scope.physicalChannels)
		return -1;
    */
	this->miscComboBox[channel]->setCurrentIndex(coupling);
	return coupling;
}

/// \brief Sets the gain for a channel.
/// \param channel The channel, whose gain should be set.
/// \param gain The gain in volts.
/// \return Index of gain-value, -1 on error.
int VoltageDock::setGain(int channel, double gain) {
    qDebug() << "VoltageDock::setGain to " << gain << endl;
	if(channel < 0 || channel >= this->settings->scope.voltage.count())
		return -1;
	
	int index = this->gainSteps.indexOf(gain);
	if(index != -1)
		this->gainComboBox[channel]->setCurrentIndex(index);
	
	return index;
}

/// \brief Sets the mode for the math channel.
/// \param mode The math-mode.
/// \return Index of math-mode, -1 on error.
int VoltageDock::setMode(DSOAnalyzer::MathMode mode) {
    qDebug() << "dockwindows: math mode changed" << endl;
    if(mode >= DSOAnalyzer::MathMode::ADD_CH1_CH2 && mode <= DSOAnalyzer::MathMode::SUB_CH2_FROM_CH1) {
        this->miscComboBox[this->settings->scope.physicalChannels]->setCurrentIndex((int)mode);
        return (int)mode;
	}
	
	return -1;
}

/// \brief Enables/disables a channel.
/// \param channel The channel, that should be enabled/disabled.
/// \param used True if the channel should be enabled, false otherwise.
/// \return Index of channel, -1 on error.
int VoltageDock::setUsed(int channel, bool used) {
    if (used)
        qDebug() << "VoltageDock::setUsed  switch channel " << channel << "on" << endl;
    else
        qDebug() << "VoltageDock::setUsed  switch channel " << channel << "off" << endl;

	if(channel >= 0 && channel < this->settings->scope.voltage.count()) {
		this->usedCheckBox[channel]->setChecked(used);
		return channel;
	}
	
	return -1;
}

/// \brief Called when the gain combo box changes it's value.
/// \param index The index of the combo box item.
void VoltageDock::gainSelected(int index) {
	int channel;
    double gainID;
	// Which combobox was it?
	for(channel = 0; channel < this->settings->scope.voltage.count(); ++channel)
		if(this->sender() == this->gainComboBox[channel])
			break;
	
	// Send signal if it was one of the comboboxes
    qDebug() << "VoltageDock::gainSelected on channel: " << channel << endl;
    if (index >this->gainSteps.size())
        return;

    qDebug() <<"Voltagedock::gainSelected no of gains: " << this->gainSteps.size() << " new gain: " << this->gainSteps.at(index) << endl;
    if(channel < this->settings->scope.voltage.count()) {
        this->settings->scope.voltage[channel].gain = this->gainSteps.at(index);
        if (m_device != nullptr) {
            const std::vector<DSO::dsoGainLevel> &gainSpecs = m_device->getGainSpecs();
            gainID=-1;
            for (int i=0; i<gainSpecs.size();++i) {
                if (gainSpecs[i].gainSteps == this->settings->scope.voltage[channel].gain) {
                    gainID = gainSpecs[i].gainIndex;
                    break;
                }
            }
            if (gainID == -1)
                qDebug() << "VoltageDock::gainSelected: gain not found in specs" <<endl;
            else {
                this->settings->scope.voltage[channel].gainID=gainID;
                qDebug() << "gain index set to " << gainID;
            }
        }
        m_device->setGain(channel,this->settings->scope.voltage[channel].gain);
        emit gainChanged(channel, this->settings->scope.voltage[channel].gain);
    }
}

/// \brief Called when the misc combo box changes it's value.
/// \param index The index of the combo box item.
void VoltageDock::miscSelected(int index) {
	int channel;
	
	// Which combobox was it?
    qDebug() << "misc selected " << endl;
	for(channel = 0; channel < this->settings->scope.voltage.count(); ++channel)
        if(this->sender() == this->miscComboBox[channel])
            break;

	// Send signal if it was one of the comboboxes
	if(channel < this->settings->scope.voltage.count()) {
		this->settings->scope.voltage[channel].misc = index;
		if(channel < (int) this->settings->scope.physicalChannels)
            emit couplingChanged(channel, (DSO::Coupling) this->settings->scope.voltage[channel].misc);
        else {
            qDebug() << "Sending mode changed with math mode: " << (int) this->settings->scope.voltage[channel].misc << endl;
            emit modeChanged((DSOAnalyzer::MathMode) this->settings->scope.voltage[channel].misc);
        }
    }
}

/// \brief Called when the used checkbox is switched.
/// \param checked The check-state of the checkbox.
void VoltageDock::usedSwitched(bool checked) {
	int channel;
	
	// Which checkbox was it?
	for(channel = 0; channel < this->settings->scope.voltage.count(); ++channel)
		if(this->sender() == this->usedCheckBox[channel])
			break;
	
	// Send signal if it was one of the checkboxes
	if(channel < this->settings->scope.voltage.count()) {
		this->settings->scope.voltage[channel].used = checked;
		emit usedChanged(channel, checked);
	}
}

void VoltageDock::updateSpecifications() {
    CurrentDevice *sender = (CurrentDevice *) QObject::sender();
//    qDebug() << "VoltageDock::updateSpecifications: update gain specs "  << endl;
    this->m_device = sender->getCurrentDevice();

    const std::vector<DSO::Coupling> availableCoupling = m_device->getAvailableCoupling();
    qDebug() << "available coupling: " << availableCoupling.size()<<" value: " << (int)availableCoupling[0] << endl;

    this->couplingStrings.clear();
    for (int i=0;i<availableCoupling.size();++i) {
        this->couplingStrings.append(Dso::couplingString(availableCoupling[i]));
        qDebug() << "Counpling: " << Dso::couplingString(availableCoupling[i]);
    }
    const std::vector<DSO::dsoGainLevel> &gainSpecs = m_device->getGainSpecs();

   qDebug() << "no of gains: " << gainSpecs.size() << endl;

   /* clear the gain steps */
   this->gainSteps.clear();
   this->gainStrings.clear();

   /* get the number of physical channels from the specs */

   this->settings->scope.physicalChannels = m_device->getPhysicalChannels();

   int defaultGainIndex = -1;

   for (int i=0;i<gainSpecs.size();i++)
       if (gainSpecs[i].gainSteps == 8) {  //this corresponds to 1 V
           defaultGainIndex = i;
           break;
       }
   if (defaultGainIndex == -1)
       defaultGainIndex = 0;

   /* now refill with the new specs */
   for (unsigned int i=0; i< gainSpecs.size();++i) {
       qDebug() << "gain: " << gainSpecs[i].gainSteps/DIVS_VOLTAGE << endl;
       qDebug() << "gainID: " << gainSpecs[i].gainIndex << endl;
       qDebug() << "gain string: " <<  Helper::valueToString((double)gainSpecs[i].gainSteps/DIVS_VOLTAGE, Helper::UNIT_VOLTS, 0) << endl;
        this->gainSteps.append(gainSpecs[i].gainSteps);
        this->gainStrings << Helper::valueToString((double)gainSpecs[i].gainSteps/DIVS_VOLTAGE, Helper::UNIT_VOLTS, 0);
    }
    /*
     * reset the gain combobox and setup the entries define in the specs for this device
     */

    for(int channel = 0; channel < this->settings->scope.voltage.size(); ++channel) {

//        this->settings->scope.physicalChannels
        /* without blocking signals the program risks to crash */
            gainComboBox[channel]->blockSignals(true);
            gainComboBox[channel]->clear();
            gainComboBox[channel]->addItems(this->gainStrings);
            gainComboBox[channel]->setCurrentIndex(defaultGainIndex);
            gainComboBox[channel]->blockSignals(false);

        qDebug() << "setting default gain on channel " << channel << " to " << gainSpecs[defaultGainIndex].gainSteps/DIVS_VOLTAGE << endl;
        this->settings->scope.voltage[channel].gain = gainSpecs[defaultGainIndex].gainSteps;
        this->settings->scope.voltage[channel].gainID = gainSpecs[defaultGainIndex].gainIndex;
        m_device->setGain(channel,this->settings->scope.voltage[channel].gain);     // set the gain on the device

        qDebug() << "gain no of entries" << gainComboBox[channel]->count() << endl;

        if (channel < m_device->getPhysicalChannels()) {
            miscComboBox[channel]->blockSignals(true);
            miscComboBox[channel]->clear();
            miscComboBox[channel]->addItems(this->couplingStrings);
            miscComboBox[channel]->setCurrentIndex(0);
            miscComboBox[channel]->blockSignals(false);
        }
        emit(gainChanged(channel,this->settings->scope.voltage[channel].gain));
    }
}

void VoltageDock::hard_event(int type, int value)
{
	int idx;

	switch (type) {
	case PANEL_SW_CH1_ENABLE:
		usedCheckBox[0]->toggle();
		break;
	case PANEL_SW_CH2_ENABLE:
		usedCheckBox[1]->toggle();
		break;
	case PANEL_SW_CH1_SCALE:
		miscComboBox[0]->setCurrentIndex(!miscComboBox[0]->currentIndex());
		break;
	case PANEL_SW_CH2_SCALE:
		miscComboBox[1]->setCurrentIndex(!miscComboBox[1]->currentIndex());
		break;
	case PANEL_ENC_CH1_SCALE:
		idx = gainComboBox[0]->currentIndex() - value;
		if (idx < 0)
			idx = 0;
		if (idx > gainComboBox[0]->count() - 1)
			idx = gainComboBox[0]->count() - 1;
		gainComboBox[0]->setCurrentIndex(idx);
		break;
	case PANEL_ENC_CH2_SCALE:
		idx = gainComboBox[1]->currentIndex() - value;
		if (idx < 0)
			idx = 0;
		if (idx > gainComboBox[1]->count() - 1)
			idx = gainComboBox[1]->count() - 1;
		gainComboBox[1]->setCurrentIndex(idx);
		break;
	}
}
