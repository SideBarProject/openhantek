#pragma once

#include <QAbstractListModel>
#include <QStringList>

#include <memory>
namespace DSO {
    class DeviceBase;
    class DeviceList;
}
class DeviceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QStringList supportedDevices READ supportedDevices NOTIFY supportedDevicesChanged)
public:
    enum DeviceRoles {
        NameRole = Qt::UserRole + 1,
        NeedFirmware,
        UIDRole
    };
    DeviceModel(const DSO::DeviceList* deviceList);

    /// Part of the model. Return the size of the model.
    int rowCount(const QModelIndex & parent = QModelIndex()) const override;

    /// Part of the model. Return part of a device entry (DSO::DeviceBase) for example
    /// the name.
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

    /// Part of the model. Maps integers to role names for QML.
    QHash<int, QByteArray> roleNames() const override;

    /// This is called by {@see DSO::DeviceList} if devices have been plugin in or out.
    void update();

    /// Return a list of all supported device models.
    const QStringList supportedDevices() const;

    Q_INVOKABLE int uploadFirmware(unsigned uid);

    /// This is called by {@see DSO::DeviceList} if more device models have been registered
    /// to it or models have been removed and supportedDevices() would return another
    /// result now.
    void updateSupportedDevices() {emit supportedDevicesChanged();}
signals:
    void countChanged();
    void supportedDevicesChanged();

private:
    const DSO::DeviceList* m_deviceList;
};
