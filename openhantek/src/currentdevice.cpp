#include "currentdevice.h"
#include "deviceList.h"
#include "libDemoDevice/sineWaveDevice.h"
#include <iostream>
#include "settings.h"
#include <QCoreApplication>
#include <QDebug>
#include "openhantek.h"
#include <unistd.h>
#include <stdio.h>

CurrentDevice::CurrentDevice(DSO::DeviceList* deviceList)
    : m_deviceList(deviceList)
{
    std::cout << "CurrentDevice: creation" << std::endl;
    /* connects signal newDataFromDataAnalyzer to updateCurves */

    std::cout << "CurrentDevice: connecting signal newDataFromDataAnalyzer to updateCurves" << std::endl;
    connect(this, &CurrentDevice::newDataFromDataAnalyzer, this, &CurrentDevice::updateCurves, Qt::QueuedConnection);
//    connect(this, SIGNAL(newDataFromDataAnalyzer()), this, SLOT(updateCurves()), Qt::QueuedConnection);

    emit newDataFromDataAnalyzer();

}
/* set the downsampling rate in the analyzer */
void CurrentDevice::setDownsampler(unsigned int downsampling) {
    this->m_analyzerSettings.downsampling = downsampling;
}

void CurrentDevice::setDevice(std::shared_ptr<DSO::DeviceBase> deviceBase)
{
    if (!deviceBase || m_device) {
        std::cout << "CurrentDevice::setDevice() m_device not existing" << std::endl;
        return;
    }
    m_device = deviceBase;
    int noOfPhysicalChannels = m_device->getChannelCount();
    m_settings->scope.physicalChannels = noOfPhysicalChannels;
    m_settings->scope.horizontal.format = DSO::GRAPHFORMAT_TY;

    // Disconnect signal: remove all function pointer connections and reset CurrentDevice
    std::cout << "register deviceDisconnected and deviceConnected in baseDevice" << std::endl;
    m_device->_deviceDisconnected = [this]() {
        this->resetDevice();
        m_device->_deviceDisconnected = [](){};
        m_device->_deviceConnected = [](){};

    };

    m_device->_deviceConnected = [this]() {
        std::cout << "CurrentDevice::setDevice create analyzer " << std::endl;
        m_analyzer = std::unique_ptr<DSOAnalyzer::DataAnalyzer>(new DSOAnalyzer::DataAnalyzer(m_device,&m_analyzerSettings));
        this->m_glGenerator->setAnalyzer(m_analyzer);
        std::cout << "CurrentDevice::setDevice connect analyzed to emitting newDataFromAnalyzer " << std::endl;
        m_analyzer->_analyzed = [this]() {updateCurves();};

        for (unsigned channel = 0; channel < m_device->getChannelCount(); ++channel) {
            m_device->setChannelUsed(channel, true);
        }

        bool mathUsed = this->m_settings->scope.voltage[this->m_settings->scope.physicalChannels].used | this->m_settings->scope.spectrum[this->m_settings->scope.physicalChannels].used;
        if (mathUsed)
            qDebug() << "setting math used in analyzer" << endl;
        else
            qDebug() << "setting math NOT used in analyzer" << endl;
        qDebug() << "sending specificattons changed" << endl;
        emit(specificationsChanged());
        m_analyzerSettings.mathChannelEnabled = mathUsed;

    };
    //        emit curvesChanged();

    std::cout << "CurrentDevice::setDevice: connecting device" << std::endl;
    m_device->connectDevice();
    std::cout <<"Finished connecting to scope " << endl;
    emit validChanged();
    emit channelsChanged(m_device->getChannelCount());
}

void CurrentDevice::setDevice(unsigned uid)
{
    setDevice(m_deviceList->getDeviceByUID(uid));
}

std::shared_ptr<DSO::DeviceBase>  CurrentDevice::getCurrentDevice()
{
    return m_device;
}

std::shared_ptr<DSOAnalyzer::DataAnalyzer> CurrentDevice::getAnalyzer()
{
    return m_analyzer;
}

void CurrentDevice::setDemoDevice()
{
    std::cout << "setDemoDevice()" << std::endl;
    setDevice(std::shared_ptr<DSO::DeviceBase>(new DemoDevices::SineWaveDevice()));
}

void CurrentDevice::resetDevice()
{
    emit channelsChanged(0);
    if (m_device)
        m_device->disconnectDevice();
    std::cout << "_analyzed is set to null function" << std::endl;
    if (m_analyzer) // Disconnect signal
        m_analyzer->_analyzed = []() {};
    m_analyzer.reset();
    m_device.reset();
//   emit validChanged();
}


void CurrentDevice::updateCurves()
{
    if (!m_analyzer) {
        qDebug() << "updateCurves: there is no analyzer" << endl;
        return;
    }
    using namespace std;
    // Check if the sample count has changed
    unsigned int sampleCount = m_analyzer->sampleCount();
    if (sampleCount == 0) {
        std::cout << "updateCurves: sample count is zero" << std::endl;
        m_analyzer->mutex().unlock();
        return;
    }


    std::cout << "update curves: sample count: " << sampleCount << std::endl;
    qDebug() << "m_settings->scope.horizontal.format" << m_settings->scope.horizontal.format << endl;

    const DSOAnalyzer::SampleValues& sampleValues = m_analyzer->data(0)->samples.voltage;
    std::cout << "get pointer to sample values" << std::endl;
    const DSOAnalyzer::AnalyzedData *mydata = m_analyzer->data(0);
//    printf("currentDevice pointer to samples %08x\n",mydata);
//    printf("currentDevice pointer to voltage samples %08x on channel 0\n",mydata->samples.voltage.sample);
    std::cout << "sample size: " << mydata->samples.voltage.sample.size() << std::endl;
//    for (int i=0;i<10;i++)
//        printf("sample[%d]: %10f4\n",mydata->samples.voltage.sample[i]);
    qDebug() << "size: " << sampleValues.sample.size() << endl;
    double interval = sampleValues.interval;
    qDebug() << "currentDevice: interval" << interval << endl;
    std::vector<double> values(sampleValues.sample.begin(),sampleValues.sample.end());

    m_glGenerator->generateGraphs();

//    if (m_settings->scope.trigger.mode == DSO::TriggerMode.SINGLE)
//        qDebug() << "Single shot mode, switch back to non sampling " << endl;

/*
    for (unsigned channel = 0; channel < m_device->getChannelCount(); ++channel) {
//        QPCurve* curve = (QPCurve*)m_curves[channel];
//        curve->setOffset(200); //-sampleCount/2);

        const DSOAnalyzer::SampleValues& sampleValues = m_analyzer->data(channel)->samples.voltage;


        // What's the horizontal distance between sampling points?
        double horizontalFactor = sampleValues.interval / m_scopeViewSettings.timebase;

        std::vector<double>::const_iterator dataIterator = sampleValues.sample.begin();
        const double gain = m_analyzerSettings.voltage[channel].gain;
        const double offset = m_analyzerSettings.voltage[channel].offset;

//        const double divs_time = m_scopeViewSettings.divs_time;

        for(unsigned int position = 0; position < sampleCount; ++position) {
            // *(glIterator++) = position * horizontalFactor - divs_time / 2; //X
               *(dataIterator) = *(dataIterator++) / gain + offset;             //Y
               std::for_each (dataIterator.begin(),dataIterator.end(), [](double& d) { d=d/gain + offset;});
        }
//       std::vector<float> fsamples(sampleValues.sample.begin(),sampleValues.sample.end());
//        QVector<float> qsamples = QVector<float>::fromStdVector(fsamples);

        //        curve->setData(sampleValues.sample);
        //       curve->setData(qsamples);
    }

*/
    m_analyzer->mutex().unlock();

}
QString CurrentDevice::getModelName(){
    return modelName;
}
void CurrentDevice::setModelName(QString model){
    this->modelName = model;
}

void CurrentDevice::setGlGenerator(GlGenerator *glGenerator) {
     m_glGenerator = glGenerator;
}

bool CurrentDevice::valid() const
{
    return m_device.get();
}
