#pragma once

#include <QObject>

#include <QList>
#include <QTimer>

#include <memory>


#include "dataAnalyzer.h"
#include "dataAnalyzerSettings.h"
#include "parameters.h"
#include "glgenerator.h"

namespace DSO {
    class DeviceBase;
    class DeviceList;
}
namespace DSOAnalyzer {
    class DataAnalyzer;
}

/**
 * @brief A wrapper for DSO::DeviceBase with QObject signals and slots.
 */
class CurrentDevice : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Creates a CurrentDevice object.
     * @param deviceList The deviceList object have to be valid as long as
     * this object is alive.
     */

    CurrentDevice(DSO::DeviceList* deviceList);

    bool valid() const;
    std::shared_ptr<DSO::DeviceBase> getCurrentDevice();
    std::shared_ptr<DSOAnalyzer::DataAnalyzer> getAnalyzer();
    DsoSettings *m_settings;
    void setGlGenerator(GlGenerator *GlGenerator);
    void setModelName(QString model);
    QString getModelName();
    void setDownsampler(unsigned int downsampling);

signals:
    void validChanged();
    void curvesChanged();
    void newDataFromDataAnalyzer();
    void channelsChanged(unsigned);
    void specificationsChanged();

public slots:

    void setDevice(std::shared_ptr<DSO::DeviceBase> deviceBase);
    void setDevice(unsigned uid);
    // Set a demo device as current device
    void setDemoDevice();
    void resetDevice();
    void updateCurves();

private:
    // The device pointer is shared with the device list.
    std::shared_ptr<DSO::DeviceBase> m_device;
    // The data analyzer. Will be created on setDevice(device).
    std::shared_ptr<DSOAnalyzer::DataAnalyzer> m_analyzer;
    // A pointer to the deviceList. The object is not owned by CurrentDevice
    DSO::DeviceList* m_deviceList;
    GlGenerator *m_glGenerator;
    QString modelName;
    // Settings

    AnalyzerSettings m_analyzerSettings;

/*
    ScopeSettings m_scopeViewSettings;
    std::vector<ChannelSettings> m_spectrumSettings; ///< Spectrum analysis settings
    std::vector<ChannelSettings> m_voltageSettings; ///< Settings for the normal graphs
    TriggerSettings m_trigger; ///< Settings for the trigger
*/

};
