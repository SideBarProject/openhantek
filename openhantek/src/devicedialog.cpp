#include <QtGui>

#include "devicedialog.h"
#include <QStringList>
#include <QStringListModel>
#include <QComboBox>
#include <thread>
#include "errorcodes.h"

DeviceDialog::DeviceDialog(DSO::DeviceList *deviceList, QWidget *parent)
    :QDialog(parent)
{
    m_deviceList = deviceList;
    connectedDevices = m_deviceList->getList();

    if (connectedDevices.size() == 0) {
        QLabel *noDeviceLabel = new QLabel("No devices found! We support the following models:");

        QPushButton *loadDemoDeviceButton = new QPushButton("Demo Mode (Sine Wave)");
        QListView *modelNames = new QListView();
        connect (loadDemoDeviceButton,SIGNAL(clicked(bool)),this,SLOT(loadDemoDevice()));

        QVBoxLayout *vlayout = new QVBoxLayout();
        vlayout->addWidget(noDeviceLabel);
        vlayout->addWidget(modelNames);
        vlayout->addWidget(loadDemoDeviceButton);
        setLayout(vlayout);

        QStringList deviceModels;

        knownModels = m_deviceList->getKnownModels();
        for (int i=0; i<knownModels.size();i++) {
            qDebug() << "models: " << endl;
            qDebug() << QString::fromStdString(knownModels[i].modelName) << endl;
            deviceModels.append(QString::fromStdString(knownModels[i].modelName));
        }
        QStringListModel *model = new QStringListModel(this);

        model ->setStringList(deviceModels);
        modelNames -> setModel(model);
    }
    else {
/*
 * here if a scope is connected
 */
        QLabel *deviceLabel = new QLabel("Connected Devices");
        connectedDevicesBox = new QComboBox();
        for (int i=0;i<connectedDevices.size();i++) {
            uid = connectedDevices[i]->getUniqueID();
            qDebug() << "uid is " << uid << endl;
            modelDescription = connectedDevices[i]->getModel();
            qDebug() << "model found: " << QString::fromStdString(modelDescription.modelName) << endl;
            connectedDevicesBox->addItem(QString::fromStdString(modelDescription.modelName));
        }

        firmwareVersions = new QComboBox();
        for (int i=0; i<modelDescription.firmwareVersionNames.size();i++)
            firmwareVersions->addItem(QString::fromStdString(modelDescription.firmwareVersionNames[i]));
        loadFirmwareButton = new QPushButton("Load Firmware");
        cancelButton = new QPushButton("connect dummy device");
        okButton     = new QPushButton("connect scope");
        if (modelDescription.need_firmware)
            okButton->setEnabled(false);
        else
            okButton->setEnabled(true);
        QGridLayout *glayout = new QGridLayout();
        glayout->addWidget(deviceLabel,0,0,1,2);
        glayout->addWidget(connectedDevicesBox,1,0,1,2);
        glayout->addWidget(firmwareVersions,2,0,1,2);
        glayout->addWidget(loadFirmwareButton,3,0,1,2);
        glayout->addWidget(cancelButton,4,0);
        glayout->addWidget(okButton,4,1);
        setLayout(glayout);
        connect (cancelButton,SIGNAL(clicked(bool)),this,SLOT(loadDemoDevice()));
        connect (loadFirmwareButton,SIGNAL(clicked(bool)),this,SLOT(loadFirmware()));
        connect (okButton,SIGNAL(clicked(bool)),this,SLOT(connectDevice()));
    }
}

QString DeviceDialog::getFirmwareFilename() {
    return (firmwareVersions->currentText());
}

void DeviceDialog::loadDemoDevice() {
  //    qDebug() << "load Demo Device" << endl;
    this->modelName = QString("Demo Device");
    this->reject();
}

void DeviceDialog::connectDevice() {
    qDebug() << "load the scope device" << endl;

    int index = connectedDevicesBox->currentIndex();
    m_deviceList->checkForDevices();

    connectedDevices = m_deviceList->getList();
    qDebug() << "no of connected devices: " << connectedDevices.size() << endl;

    modelDescription = connectedDevices[index]->getModel();
    qDebug() << "model: " << QString::fromStdString(modelDescription.modelName) << endl;
    this->modelName =  QString::fromStdString(modelDescription.modelName);

    uid = connectedDevices[index]->getUniqueID();
    qDebug() << "uid of selected device: " << uid << endl;

    this->done(uid);
}

void DeviceDialog::loadFirmware()
{
    qDebug() << "load firmware" << endl;
    qDebug() << "firmware: " << firmwareVersions->currentText() << endl;

    int index = connectedDevicesBox->currentIndex();

/*
 * download the firmware and check for errors
 */
    if (!QString::compare(firmwareVersions->currentText(),QString("Hoenicke firmware"))) {
        qDebug() << "Hoenicke firmware selected" << endl;
        connectedDevices[index]->setFirmwareFilename(std::string("firmware/hoenicke_fw.ihx"));
    }
    else {
        qDebug() << "Hantek firmware selected" << endl;
        connectedDevices[index]->setFirmwareFilename(std::string("firmware/hantek_fw.ihx"));
    }

   if (connectedDevices[index]->uploadFirmware() == ErrorCode::ERROR_NONE) {
        qDebug() << "Firmware successfully loaded" << endl;
/*
 * wait until the scope has rebooted with the new firmware
 */
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
/*
 * update the device list
 */
        m_deviceList->checkForDevices();

        connectedDevices = m_deviceList->getList();
        qDebug() << "no of connected devices: " << connectedDevices.size() << endl;

        modelDescription = connectedDevices[index]->getModel();
        qDebug() << "model: " << QString::fromStdString(modelDescription.modelName) << endl;
        connectedDevicesBox->setItemText(index,QString::fromStdString(modelDescription.modelName));
        okButton->setEnabled(true);
        uid = connectedDevices[index]->getUniqueID();
        this->setResult(uid);
    }
}

QString DeviceDialog::getModelName() {
    return this->modelName;
}
