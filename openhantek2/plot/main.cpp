#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQuick/QQuickView>

#include "qpplotitem.h"
#include "qpscaleengine.h"
#include "qpfixedscaleengine.h"
#include "qptightscaleengine.h"
#include "qpcurve.h"
#include "qpscrollingcurve.h"

int main(int argc, char *argv[])
{
//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);

    qmlRegisterType<QPPlotItem>("QuickPlot", 0, 1, "PlotItem");
    qmlRegisterType<QPScaleEngine>("QuickPlot", 0, 1, "ScaleEngine");
    qmlRegisterType<QPFixedScaleEngine>("QuickPlot", 0, 1, "FixedScaleEngine");
    qmlRegisterType<QPTightScaleEngine>("QuickPlot", 0, 1, "TightScaleEngine");
    qmlRegisterType<QPCurve>("QuickPlot", 0, 1, "Curve");
    qmlRegisterType<QPScrollingCurve>("QuickPlot", 0, 1, "ScrollingCurve");

    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);
    view.setSource(QUrl("qrc:///main.qml"));
    view.show();

    return app.exec();

}
