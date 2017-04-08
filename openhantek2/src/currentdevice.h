#pragma once

#include <QObject>
#include <QQmlListProperty>
#include <QList>
#include <QTimer>

#include <memory>


#include "dataAnalyzer.h"
#include "dataAnalyzerSettings.h"
#include "parameters.h"

namespace DSO {
    class DeviceBase;
    class DeviceList;
}
namespace DSOAnalyser {
    class DataAnalyzer;
}
class QPCurve;
//class QPScaleEngine;
#include "plot/qpscaleengine.h"

/**
 * @brief A wrapper for DSO::DeviceBase with QObject signals and slots.
 */
class CurrentDevice : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool valid READ valid NOTIFY validChanged)
    Q_PROPERTY(QQmlListProperty<QPCurve> channels READ curves NOTIFY curvesChanged)
    Q_PROPERTY(QPScaleEngine* yScaleEngine READ yScaleEngine NOTIFY yScaleEngineChanged)
    Q_PROPERTY(QPScaleEngine* xScaleEngine READ xScaleEngine NOTIFY xScaleEngineChanged)

public:
    /**
     * @brief Creates a CurrentDevice object.
     * @param deviceList The deviceList object have to be valid as long as
     * this object is alive.
     */
    CurrentDevice(DSO::DeviceList* deviceList);
    bool valid() const;

    QQmlListProperty<QPCurve> curves();

    QPScaleEngine* yScaleEngine() const;
    QPScaleEngine* xScaleEngine() const;

signals:
    void validChanged();
    void curvesChanged();
    void yScaleEngineChanged();
    void xScaleEngineChanged();
    void newDataFromDataAnalyser();
    void channelsChanged(unsigned);

public slots:
    void setDevice(std::shared_ptr<DSO::DeviceBase> deviceBase);
    void setDevice(unsigned uid);
    // Set a demo device as current device
    void setDemoDevice();
    void resetDevice();

private:
    // The device pointer is shared with the device list.
    std::shared_ptr<DSO::DeviceBase> m_device;
    // The data analyser. Will be created on setDevice(device).
    std::unique_ptr<DSOAnalyser::DataAnalyzer> m_analyser;
    // A pointer to the deviceList. The object is not owned by CurrentDevice
    DSO::DeviceList* m_deviceList;
    // A list of the plot curves
    QList<QPCurve*> m_curves;
    // The scale engines
    std::unique_ptr<QPScaleEngine> m_yScaleEngine;
    std::unique_ptr<QPScaleEngine> m_xScaleEngine;

    // Settings
    AnalyserSettings m_analyserSettings;
    ScopeSettings m_scopeViewSettings;
    std::vector<ChannelSettings> m_spectrumSettings; ///< Spectrum analysis settings
    std::vector<ChannelSettings> m_voltageSettings; ///< Settings for the normal graphs
    TriggerSettings m_trigger; ///< Settings for the trigger

    void updateCurves();
};
