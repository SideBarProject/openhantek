#include "currentdevice.h"
#include "deviceList.h"
#include "dataAnalyzer.h"

#include "libDemoDevice/sineWaveDevice.h"
#include "plot/qpscrollingcurve.h"
#include "plot/qpcurve.h"
#include "plot/qpfixedscaleengine.h"
#include <iostream>

#include <QCoreApplication>

CurrentDevice::CurrentDevice(DSO::DeviceList* deviceList)
    : m_deviceList(deviceList), m_yScaleEngine(new QPFixedScaleEngine()), m_xScaleEngine(new QPFixedScaleEngine())
{
    m_yScaleEngine->setAxis(Qt::YAxis);
    m_yScaleEngine->setMin(-1.5);  
    m_yScaleEngine->setMax( 1.5);

    m_yScaleEngine->setAxis(Qt::XAxis);
    m_xScaleEngine->setMin(-100);
    m_xScaleEngine->setMax( 100);

    connect(this, &CurrentDevice::newDataFromDataAnalyser, this, &CurrentDevice::updateCurves, Qt::QueuedConnection);
}

void CurrentDevice::setDevice(std::shared_ptr<DSO::DeviceBase> deviceBase)
{
    if (!deviceBase || m_device) return;
    m_device = deviceBase;
    // Disconnect signal: remove all function pointer connections and reset CurrentDevice
    m_device->_deviceDisconnected = [this]() {
        this->resetDevice();
        m_device->_deviceDisconnected = [](){};
        m_device->_deviceConnected = [](){};
        qDeleteAll(m_curves);
        m_curves.clear();

    };
    m_device->_deviceConnected = [this]() {
        m_analyser = std::unique_ptr<DSOAnalyser::DataAnalyzer>(new DSOAnalyser::DataAnalyzer(m_device,&m_analyserSettings));
        m_analyser->_analyzed = [this]() { emit newDataFromDataAnalyser(); };

        for (unsigned channel = 0; channel < m_device->getChannelCount(); ++channel) {
            QPCurve *item = new QPCurve();
            item->setColor(QColor::fromHsv(channel * 60, 0xff, 0xff));
            m_curves.append(item);
            m_device->setChannelUsed(channel, true);
        }

        m_device->setChannelUsed(0, true);
        emit curvesChanged();
    };
    m_device->connectDevice();
    emit validChanged();
    emit channelsChanged(m_device->getChannelCount());
}

void CurrentDevice::setDevice(unsigned uid)
{
    setDevice(m_deviceList->getDeviceByUID(uid));
}

void CurrentDevice::setDemoDevice()
{
    setDevice(std::shared_ptr<DSO::DeviceBase>(new DemoDevices::SineWaveDevice()));
}

void CurrentDevice::resetDevice()
{
    emit channelsChanged(0);
    if (m_device)
        m_device->disconnectDevice();
    if (m_analyser) // Disconnect signal
        m_analyser->_analyzed = []() {};
    m_analyser.reset();
    m_device.reset();
    emit validChanged();
}

QPScaleEngine* CurrentDevice::yScaleEngine() const
{
    return m_yScaleEngine.get();
}

QPScaleEngine*CurrentDevice::xScaleEngine() const
{
    return m_xScaleEngine.get();
}

void CurrentDevice::updateCurves()
{
    if (!m_analyser) return;
    using namespace std;
    // Check if the sample count has changed
    unsigned int sampleCount = m_analyser->sampleCount();
/*    if (m_xScaleEngine->max() != sampleCount/2) {
        m_xScaleEngine->setMin(-double(sampleCount/2));
        m_xScaleEngine->setMax( sampleCount/2);
        emit xScaleEngineChanged();
    }
    */
    qDebug() <<"scale engine max: " << m_xScaleEngine->max() << " sample count" << sampleCount << endl;
      if (m_xScaleEngine->max() != sampleCount) {
            m_xScaleEngine->setMin(0.0);
            m_xScaleEngine->setMax( sampleCount);
            emit xScaleEngineChanged();
        }

    // qDebug() << "sample count: " << sampleCount << endl;
    for (unsigned channel = 0; channel < m_device->getChannelCount(); ++channel) {
        QPCurve* curve = (QPCurve*)m_curves[channel];
        //        curve->setOffset(200); //-sampleCount/2);

        const DSOAnalyser::SampleValues& sampleValues = m_analyser->data(channel)->samples.voltage;


        // What's the horizontal distance between sampling points?
        double horizontalFactor = sampleValues.interval / m_scopeViewSettings.timebase;

        std::vector<double>::const_iterator dataIterator = sampleValues.sample.begin();
        //        const double gain = m_analyserSettings.voltage[channel].gain;
        //        const double offset = m_analyserSettings.voltage[channel].offset;

        const double gain = 1.0;
        const double offset = 0.0;
        const double divs_time = m_scopeViewSettings.divs_time;

        for(unsigned int position = 0; position < sampleCount; ++position) {
            //*(glIterator++) = position * horizontalFactor - divs_time / 2; //X
            //            *(dataIterator) = *(dataIterator++) / gain + offset;             //Y
            //            std::for_each (dataIterator.begin(),dataIterator.end(), [](double& d) { d=d/gain + offset;});
        }
        std::vector<float> fsamples(sampleValues.sample.begin(),sampleValues.sample.end());
        QVector<float> qsamples = QVector<float>::fromStdVector(fsamples);

        //        curve->setData(sampleValues.sample);
        curve->setData(qsamples);
    }
    m_analyser->mutex().unlock();
}

QQmlListProperty<QPCurve> CurrentDevice::curves()
{
    return QQmlListProperty<QPCurve>(this, m_curves);
}

bool CurrentDevice::valid() const
{
    return m_device.get();
}
