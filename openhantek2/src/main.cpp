#include <QApplication>
#include <QQmlApplicationEngine>
#include <iostream>

#include "deviceList.h"
#include "libOpenHantek2xxx-5xxx/init.h"
#include "libOpenHantek60xx/init.h"
#include "dataAnalyzer.h"
#include "scopecolors.h"
#include "modeldatasetter.h"
#include "devicemodel.h"
#include "strings/dsoerrorstrings.h"
#include "currentdevice.h"
#include "exporter.h"

#include "plot/qpscrollingcurve.h"
#include "plot/qpfixedscaleengine.h"
#include "plot/qpcurve.h"

#include <QQuickWindow>
#include <QQuickItem>
#include <QQmlContext>
#include <QDebug>
#define VERSION "0.0"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationVersion(VERSION);

    // Create engine
    QQmlApplicationEngine* engine = new QQmlApplicationEngine;
    engine->addImportPath(":/QuickPlot");
    QQmlContext *rootContext = engine->rootContext();

    ErrorStrings errorStrings;

    // Register QML classes
    qmlRegisterType<ModelDataSetter>("ModelDataSetter", 1, 0, "ModelDataSetter"); // for QAbstractItemModel::setData calls from QML
    qmlRegisterType<QPPlotItem>("QuickPlot", 1, 0, "PlotItem");       // A plot item
    qmlRegisterType<QPCurve>("QuickPlot", 1, 0, "Curve");             // A plot curve
    qmlRegisterType<QPFixedScaleEngine>("QuickPlot", 1, 0, "ScaleEngine"); // A plot scaler
    qmlRegisterType<ErrorStrings>("ErrorStrings", 1, 0, "ErrorStrings");

    DSO::DeviceList deviceList;

    CurrentDevice currentDevice(&deviceList);
    rootContext->setContextProperty("currentDevice", &currentDevice);

    DeviceModel deviceModel(&deviceList);
    rootContext->setContextProperty("deviceModel", &deviceModel);

    ScopeColors screenColors("screen");
    rootContext->setContextProperty("screenColors", &screenColors);

    currentDevice.connect(&currentDevice, &CurrentDevice::channelsChanged, &screenColors, &ScopeColors::channelsChanged);

    rootContext->setContextProperty("version", app.applicationVersion());

    // We hand over exporter to engine, no smartpointer, no other manually clean up!
    // Exporter cannot be on the stack, because the QML engine will delete it.
    Exporter* exporter = new Exporter(*engine);
    rootContext->setContextProperty("exporter", exporter);

    // Connect deviceList with model, register all known usb identifiers

    deviceList._listChanged = [&deviceModel]() {
        deviceModel.update();
    };
    deviceList._modelsChanged = [&deviceModel]() {
        deviceModel.updateSupportedDevices();
    };
//    Hantek2xxx_5xxx::registerHantek2xxx_5xxxProducts(deviceList);
    Hantek60xx::registerHantek60xxProducts(deviceList);
    deviceList.setAutoUpdate(true);
    deviceList.update();

    // Start QML engine
    engine->load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine->rootObjects().size() == 0) {
        qWarning() << "Failed to load qml file!";
        return -1;
    }

    int r = app.exec();
    delete engine;
    return r;
}
