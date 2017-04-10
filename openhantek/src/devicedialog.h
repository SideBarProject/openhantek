#ifndef DEVICEDIALOG_H
#define DEVICEDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QListView>
#include "deviceList.h"
#include "deviceBase.h"
#include "deviceDescriptionEntry.h"

class QLabel;
class QPushButton;

class DeviceDialog : public QDialog
{
        Q_OBJECT

public:
    DeviceDialog(DSO::DeviceList *deviceList, QWidget *parent = 0);
    QString getModelName();

private slots:
    void loadDemoDevice();
    void connectDevice();
    void loadFirmware();

private:
    QListView *modelNames;
    QPushButton *loadDemoDeviceButton;
    DSO::DeviceList *m_deviceList;
    std::vector<DSO::DSODeviceDescription> knownModels;
    std::vector<std::shared_ptr<DSO::DeviceBase>> connectedDevices;
    DSO::DSODeviceDescription modelDescription;
    QString modelName;
    QComboBox *connectedDevicesBox, *firmwareVersions;
    QPushButton *loadFirmwareButton;
    QPushButton *cancelButton, *okButton;
    unsigned uid;
};

#endif // DEVICEDIALOG_H
