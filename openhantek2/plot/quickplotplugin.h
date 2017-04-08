#ifndef QUICKPLOTPLUGIN_H
#define QUICKPLOTPLUGIN_H

#include <QQmlExtensionPlugin>

class QuickPlotPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);
};

#endif // QUICKPLOTPLUGIN_H
