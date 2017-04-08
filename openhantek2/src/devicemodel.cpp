#include "devicemodel.h"
#include "deviceBase.h"
#include "deviceList.h"

#include <QTimer>
#include <QDebug>
using namespace DSO;

DeviceModel::DeviceModel(const DSO::DeviceList* deviceList) : m_deviceList(deviceList)
{
    QTimer* hotplugTimer = new QTimer(this);
    hotplugTimer->setInterval(2000);
    hotplugTimer->setSingleShot(false);
    connect(hotplugTimer, &QTimer::timeout, [deviceList]() {deviceList->checkForDevices();});
    hotplugTimer->start();
}

int DeviceModel::rowCount(const QModelIndex & parent) const {
    Q_UNUSED(parent);
    return m_deviceList->getList().size();
}

QVariant DeviceModel::data(const QModelIndex & index, int role) const {
    const DeviceBase* device = m_deviceList->getList().at(index.row()).get();
    if (role == NameRole)
        return QString::fromStdString(device->getModel().modelName);
    else if (role == NeedFirmware)
        return device->needFirmware();
    else if (role == UIDRole)
        return device->getUniqueID();
    return QVariant();
}

const QStringList DeviceModel::supportedDevices() const
{
    QStringList l;
    for (const DSODeviceDescription& desc: m_deviceList->getKnownModels())
        if (!desc.need_firmware)
            l << QString::fromStdString(desc.modelName);
    return l;
}

int DeviceModel::uploadFirmware(unsigned uid)
{
    DSO::DeviceBase* device = const_cast<DSO::DeviceList*>(m_deviceList)->getDeviceByUID(uid).get();
    if (device)
        return (int)device->uploadFirmware();

    return (int)ErrorCode::ERROR_UNSUPPORTED;
}

void DeviceModel::update()
{
    beginResetModel();
    endResetModel();
    emit countChanged();
}

QHash<int, QByteArray> DeviceModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[NeedFirmware] = "needFirmware";
    roles[UIDRole] = "uid";
    return roles;
}
