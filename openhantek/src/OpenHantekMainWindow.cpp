////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  openhantek.cpp
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

// Qt GUI
#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QDir>
#include <QFileDialog>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QToolBar>
#include <QStatusBar>

#include <functional>

// Widgets
#include "configdialog.h"
#include "dockwindows.h"
#include "dsowidget.h"
#include "settings.h"

#include "OpenHantekMainWindow.h"

// translations
#include "dsoerrorstrings.h"

// libs
#include "dataAnalyzer.h"
#include "deviceBase.h"
#include "usbCommunicationQueues.h"

////////////////////////////////////////////////////////////////////////////////
// class OpenHantekMainWindow
/// \brief Initializes the gui elements of the main window.
/// \param parent The parent widget.
/// \param flags Flags for the window manager.
OpenHantekMainWindow::OpenHantekMainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags) {
    // Set application information
    QCoreApplication::setOrganizationName("paranoiacs.net");
    QCoreApplication::setOrganizationDomain("paranoiacs.net");
    QCoreApplication::setApplicationName("OpenHantek");

    // Window title
    this->setWindowIcon(QIcon(":openhantek.png"));
    this->setWindowTitle(tr("OpenHantek"));

    // Application settings
    this->settings = new OpenHantekSettings();
    this->readSettings();

    // Create dock windows before the dso widget, they fix messed up settings
    this->createDockWindows();

    // Central oszilloscope widget
    this->dsoWidget = new DsoWidget(this->settings);
    this->setCentralWidget(this->dsoWidget);

    // Subroutines for window elements
    this->createActions();
    this->createToolBars();
    this->createMenus();
    this->createStatusBar();

    // Apply the settings after the gui is initialized
    this->applySettings();

    // Update stored window size and position
    this->settings->options.window.position = this->pos();
    this->settings->options.window.size = this->size();

    // Connect all signals
    this->connectSignals();
}

/// \brief Cleans up the main window.
OpenHantekMainWindow::~OpenHantekMainWindow() {
}

/// \brief Save the settings before exiting.
/// \param event The close event that should be handled.
void OpenHantekMainWindow::closeEvent(QCloseEvent *event) {
    if(this->settings->options.alwaysSave)
        this->writeSettings();

    QMainWindow::closeEvent(event);
}

/// \brief Create the used actions.
void OpenHantekMainWindow::createActions() {
    this->openAction = new QAction(QIcon(":actions/open.png"), tr("&Open..."), this);
    this->openAction->setShortcut(tr("Ctrl+O"));
    this->openAction->setStatusTip(tr("Open saved settings"));
    connect(this->openAction, &QAction::triggered, this, &OpenHantekMainWindow::open);

    this->saveAction = new QAction(QIcon(":actions/save.png"), tr("&Save"), this);
    this->saveAction->setShortcut(tr("Ctrl+S"));
    this->saveAction->setStatusTip(tr("Save the current settings"));
    connect(this->saveAction, &QAction::triggered, this, &OpenHantekMainWindow::save);

    this->saveAsAction = new QAction(QIcon(":actions/save-as.png"), tr("Save &as..."), this);
    this->saveAsAction->setStatusTip(tr("Save the current settings to another file"));
    connect(this->saveAsAction, &QAction::triggered, this, &OpenHantekMainWindow::saveAs);

    this->printAction = new QAction(QIcon(":actions/print.png"), tr("&Print..."), this);
    this->printAction->setShortcut(tr("Ctrl+P"));
    this->printAction->setStatusTip(tr("Print the oscilloscope screen"));
    connect(this->printAction, &QAction::triggered, this->dsoWidget, &DsoWidget::print);

    this->exportAsAction = new QAction(QIcon(":actions/export-as.png"), tr("&Export as..."), this);
    this->exportAsAction->setShortcut(tr("Ctrl+E"));
    this->exportAsAction->setStatusTip(tr("Export the oscilloscope data to a file"));
    connect(this->exportAsAction, &QAction::triggered, this->dsoWidget, &DsoWidget::exportAs);

    this->exitAction = new QAction(tr("E&xit"), this);
    this->exitAction->setShortcut(tr("Ctrl+Q"));
    this->exitAction->setStatusTip(tr("Exit the application"));
    connect(this->exitAction, &QAction::triggered, this, &OpenHantekMainWindow::close);

    this->configAction = new QAction(tr("&Settings"), this);
    this->configAction->setShortcut(tr("Ctrl+S"));
    this->configAction->setStatusTip(tr("Configure the oscilloscope"));
    connect(this->configAction, &QAction::triggered, this, &OpenHantekMainWindow::config);

    this->startStopAction = new QAction(this);
    this->startStopAction->setShortcut(tr("Space"));
    this->stopped();

    this->digitalPhosphorAction = new QAction(QIcon(":actions/digitalphosphor.png"), tr("Digital &phosphor"), this);
    this->digitalPhosphorAction->setCheckable(true);
    this->digitalPhosphorAction->setChecked(this->settings->view.digitalPhosphor);
    this->digitalPhosphor(this->settings->view.digitalPhosphor);
    connect(this->digitalPhosphorAction, &QAction::toggled, this, &OpenHantekMainWindow::digitalPhosphor);

    this->zoomAction = new QAction(QIcon(":actions/zoom.png"), tr("&Zoom"), this);
    this->zoomAction->setCheckable(true);
    this->zoomAction->setChecked(this->settings->view.zoom);
    this->zoom(this->settings->view.zoom);
    connect(this->zoomAction, &QAction::toggled, this, &OpenHantekMainWindow::zoom);
    connect(this->zoomAction, &QAction::toggled, this->dsoWidget, &DsoWidget::updateZoom);

    this->aboutAction = new QAction(tr("&About"), this);
    this->aboutAction->setStatusTip(tr("Show information about this program"));
    connect(this->aboutAction, &QAction::triggered, this, &OpenHantekMainWindow::about);

    this->aboutQtAction = new QAction(tr("About &Qt"), this);
    this->aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(this->aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);

#ifdef DEBUG
    this->commandAction = new QAction(tr("Send command"), this);
    this->commandAction->setShortcut(tr("Shift+C"));
#endif
}

/// \brief Create the menus and menuitems.
void OpenHantekMainWindow::createMenus() {
    this->fileMenu = this->menuBar()->addMenu(tr("&File"));
    this->fileMenu->addAction(this->openAction);
    this->fileMenu->addAction(this->saveAction);
    this->fileMenu->addAction(this->saveAsAction);
    this->fileMenu->addSeparator();
    this->fileMenu->addAction(this->printAction);
    this->fileMenu->addAction(this->exportAsAction);
    this->fileMenu->addSeparator();
    this->fileMenu->addAction(this->exitAction);

    this->viewMenu = this->menuBar()->addMenu(tr("&View"));
    this->viewMenu->addAction(this->digitalPhosphorAction);
    this->viewMenu->addAction(this->zoomAction);
    this->viewMenu->addSeparator();
    this->dockMenu = this->viewMenu->addMenu(tr("&Docking windows"));
    this->dockMenu->addAction(this->horizontalDock->toggleViewAction());
    this->dockMenu->addAction(this->spectrumDock->toggleViewAction());
    this->dockMenu->addAction(this->triggerDock->toggleViewAction());
    this->dockMenu->addAction(this->voltageDock->toggleViewAction());
    this->toolbarMenu = this->viewMenu->addMenu(tr("&Toolbars"));
    this->toolbarMenu->addAction(this->fileToolBar->toggleViewAction());
    this->toolbarMenu->addAction(this->oscilloscopeToolBar->toggleViewAction());
    this->toolbarMenu->addAction(this->viewToolBar->toggleViewAction());

    this->oscilloscopeMenu = this->menuBar()->addMenu(tr("&Oscilloscope"));
    this->oscilloscopeMenu->addAction(this->configAction);
    this->oscilloscopeMenu->addSeparator();
    this->oscilloscopeMenu->addAction(this->startStopAction);
#ifdef DEBUG
    this->oscilloscopeMenu->addSeparator();
    this->oscilloscopeMenu->addAction(this->commandAction);
#endif

    this->menuBar()->addSeparator();

    this->helpMenu = this->menuBar()->addMenu(tr("&Help"));
    this->helpMenu->addAction(this->aboutAction);
    this->helpMenu->addAction(this->aboutQtAction);
}

/// \brief Create the toolbars and their buttons.
void OpenHantekMainWindow::createToolBars() {
    this->fileToolBar = new QToolBar(tr("File"));
    this->fileToolBar->setAllowedAreas(Qt::TopToolBarArea);
    this->fileToolBar->addAction(this->openAction);
    this->fileToolBar->addAction(this->saveAction);
    this->fileToolBar->addAction(this->saveAsAction);
    this->fileToolBar->addSeparator();
    this->fileToolBar->addAction(this->printAction);
    this->fileToolBar->addAction(this->exportAsAction);

    this->oscilloscopeToolBar = new QToolBar(tr("Oscilloscope"));
    this->oscilloscopeToolBar->addAction(this->startStopAction);

    this->viewToolBar = new QToolBar(tr("View"));
    this->viewToolBar->addAction(this->digitalPhosphorAction);
    this->viewToolBar->addAction(this->zoomAction);
}

/// \brief Create the status bar.
void OpenHantekMainWindow::createStatusBar() {
#ifdef DEBUG
    // Command field inside the status bar
    this->commandEdit = new QLineEdit();
    this->commandEdit->hide();

    this->statusBar()->addPermanentWidget(this->commandEdit, 1);
#endif

    this->statusBar()->showMessage(tr("Ready"));

#ifdef DEBUG
    connect(this->commandAction, &QAction::triggered, [this]() {
        this->commandEdit->show();
        this->commandEdit->setFocus();
    });
    connect(this->commandEdit, &QLineEdit::returnPressed, this, &OpenHantekMainWindow::sendCommand);
#endif
}

/// \brief Create all docking windows.
void OpenHantekMainWindow::createDockWindows()
{
    this->horizontalDock = new HorizontalDock(this->settings);
    this->triggerDock = new TriggerDock(this->settings);
    this->spectrumDock = new SpectrumDock(this->settings);
    this->voltageDock = new VoltageDock(this->settings);
}

/// \brief Connect general signals and device management signals.
void OpenHantekMainWindow::connectSignals() {
    // Connect general signals
    connect(this, &OpenHantekMainWindow::settingsChanged, this, &OpenHantekMainWindow::applySettings);

    // Connect signals to DSO controller and widget
    connect(this->horizontalDock, &HorizontalDock::samplerateChanged, this, &OpenHantekMainWindow::samplerateSelected);
    connect(this->horizontalDock, &HorizontalDock::timebaseChanged, this, &OpenHantekMainWindow::timebaseSelected);
    connect(this->horizontalDock, &HorizontalDock::frequencybaseChanged, this->dsoWidget, &DsoWidget::updateFrequencybase);
    connect(this->horizontalDock, &HorizontalDock::recordLengthChanged, this, &OpenHantekMainWindow::recordLengthSelected);

    connect(this->voltageDock, &VoltageDock::usedChanged, this, &OpenHantekMainWindow::updateUsed);
    connect(this->voltageDock, &VoltageDock::usedChanged, this->dsoWidget, &DsoWidget::updateVoltageUsed);
    connect(this->voltageDock, &VoltageDock::couplingChanged, this->dsoWidget, &DsoWidget::updateVoltageCoupling);
    connect(this->voltageDock, &VoltageDock::modeChanged, this->dsoWidget, &DsoWidget::updateMathMode);
    connect(this->voltageDock, &VoltageDock::gainChanged, this, &OpenHantekMainWindow::updateVoltageGain);
    connect(this->voltageDock, &VoltageDock::gainChanged, this->dsoWidget, &DsoWidget::updateVoltageGain);
    connect(this->dsoWidget, &DsoWidget::offsetChanged, this, &OpenHantekMainWindow::updateOffset);

    connect(this->spectrumDock, &SpectrumDock::usedChanged, this, &OpenHantekMainWindow::updateUsed);
    connect(this->spectrumDock, &SpectrumDock::usedChanged, this->dsoWidget, &DsoWidget::updateSpectrumUsed);
    connect(this->spectrumDock, &SpectrumDock::magnitudeChanged, this->dsoWidget, &DsoWidget::updateSpectrumMagnitude);

    connect(this->triggerDock, &TriggerDock::modeChanged, this->dsoWidget, &DsoWidget::updateTriggerMode);
    connect(this->triggerDock, &TriggerDock::sourceChanged, this->dsoWidget, &DsoWidget::updateTriggerSource);
    connect(this->triggerDock, &TriggerDock::slopeChanged, this->dsoWidget, &DsoWidget::updateTriggerSlope);

}

/// \brief Initialize the device with the current settings.
void OpenHantekMainWindow::setDevice(std::shared_ptr<DSO::DeviceBase> device) {
    using namespace std::placeholders;

    _device = device;
    this->dataAnalyzer = std::make_shared<DSOAnalyser::DataAnalyzer>(device, &(this->settings->scope));
    this->dsoWidget->setDataAnalyzer(dataAnalyzer);
    this->settings->setChannelCount(_device->getChannelCount());
    this->triggerDock->setChannels(_device->getSpecialTriggerSources());

    connect(this->startStopAction, &QAction::triggered,
        std::bind(&DSO::DeviceBase::toogleSampling, _device));

    connect(this->voltageDock, &VoltageDock::couplingChanged,
        std::bind(&DSO::DeviceBase::setCoupling, _device, _1, _2));

    connect(this->triggerDock, &TriggerDock::modeChanged,
        std::bind(&DSO::DeviceBase::setTriggerMode, _device, _1));
    connect(this->triggerDock, &TriggerDock::sourceChanged,
         std::bind(&DSO::DeviceBase::setTriggerSource, _device, _1, _2));
    connect(this->dsoWidget, &DsoWidget::triggerPositionChanged,
         std::bind(&DSO::DeviceBase::setPretriggerPosition, _device, _1));
    connect(this->triggerDock, &TriggerDock::slopeChanged,
        std::bind(&DSO::DeviceBase::setTriggerSlope, _device, _1));
    connect(this->dsoWidget, &DsoWidget::triggerLevelChanged,
        std::bind(&DSO::DeviceBase::setTriggerLevel, _device, _1, _2));

    _device->_statusMessage = [this](int code) {
        this->statusBar()->showMessage(getErrorString(code));
    };

    // Started/stopped signals from oscilloscope
    _device->_samplingStarted = std::bind(&OpenHantekMainWindow::started, this);
    _device->_samplingStopped = std::bind(&OpenHantekMainWindow::stopped, this);

    _device->_recordTimeChanged = std::bind(&OpenHantekMainWindow::recordTimeChanged, this, std::placeholders::_1);
    _device->_samplerateChanged = std::bind(&OpenHantekMainWindow::samplerateChanged, this, std::placeholders::_1);

    _device->_recordLengthChanged = std::bind(&HorizontalDock::availableRecordLengthsChanged, this->horizontalDock, std::placeholders::_1, std::placeholders::_2);
    _device->_samplerateLimitsChanged = std::bind(&HorizontalDock::samplerateLimitsChanged, this->horizontalDock, std::placeholders::_1, std::placeholders::_2);

    _device->connectDevice();

    for(unsigned int channel = 0; channel < _device->getChannelCount(); ++channel) {
        _device->setCoupling(channel, this->settings->scope.voltage[channel].coupling);
        this->updateVoltageGain(channel);
        this->updateOffset(channel);
        _device->setTriggerLevel(channel, this->settings->scope.voltage[channel].trigger);
    }
    this->updateUsed(_device->getChannelCount());
    if(this->settings->scope.horizontal.samplerateSet)
        this->samplerateSelected();
    else
        this->timebaseSelected();
    if(!_device->getAvailableRecordLengths()->size())
        _device->setRecordLength(this->settings->scope.horizontal.recordLength);
    else {
        int index = indexOf(*_device->getAvailableRecordLengths(), this->settings->scope.horizontal.recordLength);
        _device->setRecordLength(index < 0 ? 1 : index);
    }
    _device->setTriggerMode(this->settings->scope.trigger.mode);
    _device->setPretriggerPosition(this->settings->scope.trigger.position * this->settings->scope.horizontal.timebase * DIVS_TIME);
    _device->setTriggerSlope(this->settings->scope.trigger.slope);
    _device->setTriggerSource(this->settings->scope.trigger.special, this->settings->scope.trigger.source);

    _device->startSampling();
}

/// \brief Read the settings from an ini file.
/// \param fileName Optional filename to export the settings to a specific file.
/// \return 0 on success, negative on error.
int OpenHantekMainWindow::readSettings(const QString &fileName) {
    int status = this->settings->load(fileName);

    if(status == 0)
        emit(settingsChanged());

    return status;
}

/// \brief Save the settings to the harddisk.
/// \param fileName Optional filename to read the settings from an ini file.
/// \return 0 on success, negative on error.
int OpenHantekMainWindow::writeSettings(const QString &fileName) {
    this->updateSettings();

    return this->settings->saveAs(fileName);
}

/// \brief Called everytime the window is moved.
/// \param event The move event, it isn't used here though.
void OpenHantekMainWindow::moveEvent(QMoveEvent *event) {
    Q_UNUSED(event);

    this->settings->options.window.position = this->pos();
}

/// \brief Called everytime the window is resized.
/// \param event The resize event, it isn't used here though.
void OpenHantekMainWindow::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);

    this->settings->options.window.size = this->size();
}

/// \brief Open a configuration file.
/// \return 0 on success, 1 on user abort, negative on error.
int OpenHantekMainWindow::open() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("Settings (*.ini)"));

    if(!fileName.isEmpty())
        return this->readSettings(fileName);
    else
        return 1;
}

/// \brief Save the current configuration to a file.
/// \return 0 on success, negative on error.
int OpenHantekMainWindow::save() {
    if (this->currentFile.isEmpty())
        return saveAs();
    else
        return this->writeSettings(this->currentFile);
}

/// \brief Save the configuration to another filename.
/// \return 0 on success, 1 on user abort, negative on error.
int OpenHantekMainWindow::saveAs() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save settings"), "", tr("Settings (*.ini)"));
    if (fileName.isEmpty())
        return 1;

    int status = this->writeSettings(fileName);

    if(status == 0)
        this->currentFile = fileName;

    return status;
}

/// \brief The oscilloscope started sampling.
void OpenHantekMainWindow::started() {
    this->startStopAction->setText(tr("&Stop"));
    this->startStopAction->setIcon(QIcon(":actions/stop.png"));
    this->startStopAction->setStatusTip(tr("Stop the oscilloscope"));
}

/// \brief The oscilloscope stopped sampling.
void OpenHantekMainWindow::stopped() {
    this->startStopAction->setText(tr("&Start"));
    this->startStopAction->setIcon(QIcon(":actions/start.png"));
    this->startStopAction->setStatusTip(tr("Start the oscilloscope"));
}

/// \brief Configure the oscilloscope.
void OpenHantekMainWindow::config() {
    this->updateSettings();

    DsoConfigDialog configDialog(this->settings, this);
    if(configDialog.exec() == QDialog::Accepted)
        this->settingsChanged();
}

/// \brief Enable/disable digital phosphor.
void OpenHantekMainWindow::digitalPhosphor(bool enabled) {
    this->settings->view.digitalPhosphor = enabled;

    if(this->settings->view.digitalPhosphor)
        this->digitalPhosphorAction->setStatusTip(tr("Disable fading of previous graphs"));
    else
        this->digitalPhosphorAction->setStatusTip(tr("Enable fading of previous graphs"));
}

/// \brief Show/hide the magnified scope.
void OpenHantekMainWindow::zoom(bool enabled) {
    this->settings->view.zoom = enabled;

    if(this->settings->view.zoom)
        this->zoomAction->setStatusTip(tr("Hide magnified scope"));
    else
        this->zoomAction->setStatusTip(tr("Show magnified scope"));
}

/// \brief Show the about dialog.
void OpenHantekMainWindow::about() {
    QMessageBox::about(this, tr("About OpenHantek %1").arg(VERSION), tr(
        "<p>This is a open source software for Hantek USB oscilloscopes.</p>"
        "<p>Copyright &copy; 2010, 2011 Oliver Haag &lt;oliver.haag@gmail.com&gt;</p>"
    ));
}

/// \brief The settings have changed.
void OpenHantekMainWindow::applySettings() {
    // Main window
    if(!this->settings->options.window.position.isNull())
        this->move(this->settings->options.window.position);
    if(!this->settings->options.window.size.isNull())
        this->resize(this->settings->options.window.size);

    // Docking windows
    QList<QDockWidget *> docks;
    docks.append(this->horizontalDock);
    docks.append(this->triggerDock);
    docks.append(this->voltageDock);
    docks.append(this->spectrumDock);

    QList<OpenHantekSettingsOptionsWindowPanel *> dockSettings;
    dockSettings.append(&(this->settings->options.window.dock.horizontal));
    dockSettings.append(&(this->settings->options.window.dock.trigger));
    dockSettings.append(&(this->settings->options.window.dock.voltage));
    dockSettings.append(&(this->settings->options.window.dock.spectrum));

    QList<int> dockedWindows[2]; // Docks docked on the sides of the main window

    for(int dockId = 0; dockId < docks.size(); ++dockId) {
        docks[dockId]->setVisible(dockSettings[dockId]->visible);
        if(!dockSettings[dockId]->position.isNull()) {
            if(dockSettings[dockId]->floating) {
                this->addDockWidget(Qt::RightDockWidgetArea, docks[dockId]);
                docks[dockId]->setFloating(dockSettings[dockId]->floating);
                docks[dockId]->move(dockSettings[dockId]->position);
            }
            else {
                // Check in which order the docking windows where placed
                int side = (dockSettings[dockId]->position.x() < this->settings->options.window.size.width() / 2) ? 0 : 1;
                int index = 0;
                while(index < dockedWindows[side].size() && dockSettings[dockedWindows[side][index]]->position.y() <= dockSettings[dockId]->position.y())
                    ++index;
                dockedWindows[side].insert(index, dockId);
            }
        }
        else {
            this->addDockWidget(Qt::RightDockWidgetArea, docks[dockId]);
        }
    }

    // Put the docked docking windows into the main window
    for(int position = 0; position < dockedWindows[0].size(); ++position)
        this->addDockWidget(Qt::LeftDockWidgetArea, docks[dockedWindows[0][position]]);
    for(int position = 0; position < dockedWindows[1].size(); ++position)
        this->addDockWidget(Qt::RightDockWidgetArea, docks[dockedWindows[1][position]]);

    // Toolbars
    QList<QToolBar *> toolbars;
    toolbars.append(this->fileToolBar);
    toolbars.append(this->oscilloscopeToolBar);
    toolbars.append(this->viewToolBar);

    QList<OpenHantekSettingsOptionsWindowPanel *> toolbarSettings;
    toolbarSettings.append(&(this->settings->options.window.toolbar.file));
    toolbarSettings.append(&(this->settings->options.window.toolbar.oscilloscope));
    toolbarSettings.append(&(this->settings->options.window.toolbar.view));

    QList<int> dockedToolbars; // Docks docked on the sides of the main window

    for(int toolbarId = 0; toolbarId < toolbars.size(); ++toolbarId) {
        toolbars[toolbarId]->setVisible(toolbarSettings[toolbarId]->visible);
        //toolbars[toolbarId]->setFloating(toolbarSettings[toolbarId]->floating); // setFloating missing, a bug in Qt?
        if(!toolbarSettings[toolbarId]->position.isNull() && !toolbarSettings[toolbarId]->floating) {
            /*if(toolbarSettings[toolbarId]->floating) {
                toolbars[toolbarId]->move(toolbarSettings[toolbarId]->position);
            }
            else*/ {
                // Check in which order the toolbars where placed
                int index = 0;
                while(index < dockedToolbars.size() && toolbarSettings[dockedToolbars[index]]->position.x() <= toolbarSettings[toolbarId]->position.x())
                    ++index;
                dockedToolbars.insert(index, toolbarId);
            }
        }
        else {
            this->addToolBar(toolbars[toolbarId]);
        }
    }

    // Put the docked toolbars into the main window
    for(int position = 0; position < dockedToolbars.size(); ++position)
        this->addToolBar(toolbars[dockedToolbars[position]]);
}

/// \brief Update the window layout in the settings.
void OpenHantekMainWindow::updateSettings() {
    // Main window
    this->settings->options.window.position = this->pos();
    this->settings->options.window.size = this->size();

    // Docking windows
    QList<QDockWidget *> docks;
    docks.append(this->horizontalDock);
    docks.append(this->spectrumDock);
    docks.append(this->triggerDock);
    docks.append(this->voltageDock);

    QList<OpenHantekSettingsOptionsWindowPanel *> dockSettings;
    dockSettings.append(&(this->settings->options.window.dock.horizontal));
    dockSettings.append(&(this->settings->options.window.dock.spectrum));
    dockSettings.append(&(this->settings->options.window.dock.trigger));
    dockSettings.append(&(this->settings->options.window.dock.voltage));

    for(int dockId = 0; dockId < docks.size(); ++dockId) {
        dockSettings[dockId]->floating = docks[dockId]->isFloating();
        dockSettings[dockId]->position = docks[dockId]->pos();
        dockSettings[dockId]->visible = docks[dockId]->isVisible();
    }

    // Toolbars
    QList<QToolBar *> toolbars;
    toolbars.append(this->fileToolBar);
    toolbars.append(this->oscilloscopeToolBar);
    toolbars.append(this->viewToolBar);

    QList<OpenHantekSettingsOptionsWindowPanel *> toolbarSettings;
    toolbarSettings.append(&(this->settings->options.window.toolbar.file));
    toolbarSettings.append(&(this->settings->options.window.toolbar.oscilloscope));
    toolbarSettings.append(&(this->settings->options.window.toolbar.view));

    for(int toolbarId = 0; toolbarId < toolbars.size(); ++toolbarId) {
        toolbarSettings[toolbarId]->floating = toolbars[toolbarId]->isFloating();
        toolbarSettings[toolbarId]->position = toolbars[toolbarId]->pos();
        toolbarSettings[toolbarId]->visible = toolbars[toolbarId]->isVisible();
    }
}

/// \brief The oscilloscope changed the record time.
/// \param duration The new record time duration in seconds.
void OpenHantekMainWindow::recordTimeChanged(double duration) {
    if (!_device) return;
    if(this->settings->scope.horizontal.samplerateSet && this->settings->scope.horizontal.recordLength != UINT_MAX) {
        // The samplerate was set, let's adapt the timebase accordingly
        this->settings->scope.horizontal.timebase = duration / DIVS_TIME;
        this->horizontalDock->setTimebase(this->settings->scope.horizontal.timebase);
    }

    // The trigger position should be kept at the same place but the timebase has changed
    _device->setPretriggerPosition(this->settings->scope.trigger.position * this->settings->scope.horizontal.timebase * DIVS_TIME);

    this->dsoWidget->updateTimebase(this->settings->scope.horizontal.timebase);
}

/// \brief The oscilloscope changed the samplerate.
/// \param samplerate The new samplerate in samples per second.
void OpenHantekMainWindow::samplerateChanged(double samplerate) {
    if(!this->settings->scope.horizontal.samplerateSet && this->settings->scope.horizontal.recordLength != UINT_MAX) {
        // The timebase was set, let's adapt the samplerate accordingly
        this->settings->scope.horizontal.samplerate = samplerate;
        this->horizontalDock->setSamplerate(samplerate);
    }

    this->dsoWidget->updateSamplerate(samplerate);
}

/// \brief Apply new record length to settings.
/// \param recordLength The selected record length in samples.
void OpenHantekMainWindow::recordLengthSelected(unsigned long recordLength) {
    if (!_device) return;
    _device->setRecordLength(recordLength);
}

/// \brief Sets the samplerate of the oscilloscope.
void OpenHantekMainWindow::samplerateSelected() {
    if (!_device) return;
    _device->setSamplerate(this->settings->scope.horizontal.samplerate);
}

/// \brief Sets the record time of the oscilloscope.
void OpenHantekMainWindow::timebaseSelected() {
    if (!_device) return;
    _device->setSamplerateByRecordTime(this->settings->scope.horizontal.timebase * DIVS_TIME);
}

/// \brief Sets the offset of the oscilloscope for the given channel.
/// \param channel The channel that got a new offset.
void OpenHantekMainWindow::updateOffset(unsigned int channel) {
    if (!_device) return;
    if(channel >= _device->getChannelCount())
        return;

    _device->setOffset(channel, (this->settings->scope.voltage[channel].offset / DIVS_VOLTAGE) + 0.5);
}

/// \brief Sets the state of the given oscilloscope channel.
/// \param channel The channel whose state has changed.
void OpenHantekMainWindow::updateUsed(unsigned int channel) {
    if (!_device) return;
    if(channel >= (unsigned int) this->settings->scope.voltage.size())
        return;

    bool mathUsed = this->settings->scope.voltage[_device->getChannelCount()].used | this->settings->scope.spectrum[_device->getChannelCount()].used;

    // Normal channel, check if voltage/spectrum or math channel is used
    if(channel < _device->getChannelCount())
        _device->setChannelUsed(channel, mathUsed | this->settings->scope.voltage[channel].used | this->settings->scope.spectrum[channel].used);
    // Math channel, update all channels
    else if(channel == _device->getChannelCount()) {
        for(unsigned int channelCounter = 0; channelCounter < _device->getChannelCount(); ++channelCounter)
            _device->setChannelUsed(channelCounter, mathUsed | this->settings->scope.voltage[channelCounter].used | this->settings->scope.spectrum[channelCounter].used);
    }
}

/// \brief Sets the gain of the oscilloscope for the given channel.
/// \param channel The channel that got a new gain value.
void OpenHantekMainWindow::updateVoltageGain(unsigned int channel) {
    if (!_device) return;
    if(channel >= _device->getChannelCount())
        return;

    _device->setGain(channel, this->settings->scope.voltage[channel].gain * DIVS_VOLTAGE);
}

#ifdef DEBUG
/// \brief Send the command in the commandEdit to the oscilloscope.
void OpenHantekMainWindow::sendCommand() {
    ErrorCode errorCode = dynamic_cast<DSO::CommunicationThreadQueues*>(_device.get())->stringCommand(this->commandEdit->text().toStdString());

    this->commandEdit->hide();
    this->commandEdit->clear();

    if(errorCode != ErrorCode::ERROR_NONE)
        this->statusBar()->showMessage(tr("Invalid command"), 3000);
}
#endif
