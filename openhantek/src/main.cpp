////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  main.cpp
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
#include <QLibraryInfo>
#include <QLocale>
#include <QTranslator>
#include <QDebug>
#include <iostream>

#include "openhantek.h"
#include "deviceList.h"
#include "currentdevice.h"
#include "scopecolors.h"
#include "devicemodel.h"
#include "libOpenHantek60xx/init.h"
#include "dataAnalyzerSettings.h"
#include "dataAnalyzer.h"
#include "devicedialog.h"
#include"deviceBase.h"

/// \brief Initialize resources and translations and show the main window.
int main(int argc, char *argv[]) {
	Q_INIT_RESOURCE(application);

	QApplication openHantekApplication(argc, argv);
    qDebug() << "openhantek started" << endl;
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	openHantekApplication.installTranslator(&qtTranslator);

	QTranslator openHantekTranslator;
	openHantekTranslator.load("openhantek_" + QLocale::system().name(), QMAKE_TRANSLATIONS_PATH);
	openHantekApplication.installTranslator(&openHantekTranslator);
    qDebug() << "Translator installed" << endl;

    DSO::DeviceList deviceList;

    CurrentDevice currentDevice(&deviceList);

 //   currentDevice.m_settings = openHantekMainWindow->settings;

    DeviceModel deviceModel(&deviceList);
    // Connect deviceList with model, register all known usb identifiers
    deviceList._listChanged = [&deviceModel]() {
        deviceModel.update();
    };
    deviceList._modelsChanged = [&deviceModel]() {
        deviceModel.updateSupportedDevices();
    };
    qDebug() << "register HT6022 models" << endl;
    Hantek60xx::registerHantek60xxProducts(deviceList);

    deviceList.setAutoUpdate(true);
    deviceList.update();

    qDebug() << "no of connected devices: " << deviceList.getList().size() << endl;

    OpenHantekMainWindow *openHantekMainWindow = new OpenHantekMainWindow(&currentDevice);
    qDebug() << "main window created" << endl;

    currentDevice.m_settings = openHantekMainWindow->settings;

//
// bring up the device selection dialog
//

    DeviceDialog *deviceDialog = new DeviceDialog(&deviceList);
    int retCode = deviceDialog->exec();
    std::cout << "return code: " << retCode << std::endl;
    if (retCode == QDialog::Rejected) {
        std::cout << "openhantek: setup the demo device" << std::endl;
        currentDevice.setModelName(QString("DemoDevice"));
        currentDevice.setDemoDevice();
    }
    else {
        qDebug() << "openhantek: setup scope retCode: " << retCode << endl;
        currentDevice.setModelName(deviceDialog->getModelName());
        currentDevice.setDevice(deviceList.getDeviceByUID((unsigned) retCode));
    }
    openHantekMainWindow->setAnalyzer(currentDevice.getAnalyzer());
    openHantekMainWindow->setDevice(currentDevice.getCurrentDevice());
    openHantekMainWindow->startScope();
    QString title = QString("OpenHantek on ").append(deviceDialog->getModelName());
    openHantekMainWindow->setWindowTitle(title);
    openHantekMainWindow->show();

	return openHantekApplication.exec();
}
