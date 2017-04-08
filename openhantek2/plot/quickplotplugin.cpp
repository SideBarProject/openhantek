#include <QtQml/qqml.h>
#include "quickplotplugin.h"

#include "qpplotitem.h"
#include "qpscrollingcurve.h"
#include "qpcurve.h"
#include "qpscaleengine.h"
#include "qpfixedscaleengine.h"
#include "qptightscaleengine.h"

void QuickPlotPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("QuickPlot"));
    qmlRegisterType<QPPlotItem>("QuickPlot", 1, 0, "PlotItem");
    qmlRegisterType<QPScrollingCurve>("QuickPlot", 1, 0, "ScrollingCurve");
    qmlRegisterType<QPCurve>("QuickPlot", 1, 0, "Curve");
    qmlRegisterType<QPScaleEngine>("QuickPlot", 1, 0, "ScaleEngine");
    qmlRegisterType<QPFixedScaleEngine>("QuickPlot", 1, 0, "FixedScaleEngine");
    qmlRegisterType<QPTightScaleEngine>("QuickPlot", 1, 0, "TightScaleEngine");
}
