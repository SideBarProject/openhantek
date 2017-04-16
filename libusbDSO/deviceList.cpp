#include "deviceList.h"
#include <libusb-1.0/libusb.h>
#include <iostream>

#include "deviceDescriptionEntry.h"

namespace DSO {

DeviceList::DeviceList() {
    std::cout << "creating device list" << std::endl;
    std::cout << "initializing usb" << std::endl;
    libusb_init(&_usb_context);
}

DeviceList::~DeviceList() {
    setAutoUpdate(false);
    libusb_exit(_usb_context);
}

void DeviceList::registerModel(const DSODeviceDescription& model) {
    std::cout << "deviceList: RegisterModel " << model.modelName << std::endl;
    _registeredModels.push_back(model);
    _modelsChanged();
}

int hotplug_callback_fn(libusb_context *ctx, libusb_device *device, libusb_hotplug_event event, void *user_data) {
    if (event==LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED)
        ((DeviceList*)user_data)->hotplugAdd(device);
    else
        ((DeviceList*)user_data)->hotplugRemove(device);

    return 0; // if we return 1 this callback is deregistered.
}

void DeviceList::hotplugAdd(libusb_device *device) {
    uint8_t uniqueID = libusb_get_port_number(device);
    // Check if device is connected and known already
    for(auto itr = _deviceList.begin(); itr != _deviceList.end();++itr) {
        if (itr->get()->getUniqueID() == uniqueID) {
            return;
        }
    }

    // Filter device
    libusb_device_descriptor descriptor;
    if(libusb_get_device_descriptor(device, &(descriptor)) < 0)
        return;

    const DSODeviceDescription* foundModel = nullptr;

    for(const DSODeviceDescription& model: _registeredModels) {
        // Check VID and PID
        if(descriptor.idVendor != model.vendorID)
            continue;
        if(descriptor.idProduct != model.productID)
            continue;

        foundModel = &model;
        break;
    }

    if (!foundModel) {
        /*
        std::cout << "Product Id: 0x" << std::hex << descriptor.idProduct << std::endl;
        std::cout << "VendorId: 0x" << std::hex << descriptor.idVendor << std::endl;
        std::cout << "is not a known Hantek Model" << std::endl;
        */
        return;
    }
    std::cout << "Found hantek scope with Product Id: " << std::hex << descriptor.idProduct << " and Vendor Id: " << std::hex << descriptor.idVendor << std::endl;
    std::cout << "Found device at "
              << (unsigned)libusb_get_bus_number(device)
              << " " << (unsigned)libusb_get_device_address(device)
              << " " << foundModel->modelName
              << " " << foundModel->need_firmware << std::endl;

    // Add device
    DeviceBase* d = foundModel->createDevice(device, *foundModel);
    _deviceList.push_back(std::unique_ptr<DeviceBase>(d));
    _listChanged();
}

void DeviceList::hotplugRemove(libusb_device *device) {
    uint8_t uniqueID = libusb_get_port_number(device);
    bool changed = false;
    for(auto itr = _deviceList.begin(); itr != _deviceList.end();) {
        if (itr->get()->getUniqueID() == uniqueID) {
            itr = _deviceList.erase(itr);
            changed = true;
        }
    }

    if (changed)
        _listChanged();
}

void DeviceList::checkForDevices() const
{
    struct timeval t = {0,0};
    std::cout << "deviceList: checkForDevices()" << std::endl;
    libusb_handle_events_timeout_completed(nullptr, &t, nullptr);
}

void DeviceList::setAutoUpdate(bool autoUpdate) {
    // Unregister callback before doing anything else
    if (_autoUpdate && _callback_handle) {
        libusb_hotplug_deregister_callback(nullptr, _callback_handle);
        _callback_handle = 0;
    }
    _autoUpdate = autoUpdate;
    if (_autoUpdate) {
        int err;
        err = libusb_hotplug_register_callback(nullptr,
                                         libusb_hotplug_event(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED|LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT),
                                         libusb_hotplug_flag(0), // no flags
                                         LIBUSB_HOTPLUG_MATCH_ANY, // vendor
                                         LIBUSB_HOTPLUG_MATCH_ANY, // product
                                         LIBUSB_HOTPLUG_MATCH_ANY, // usb device class
                                         hotplug_callback_fn,
                                         this,
                                         &_callback_handle);

        if (err != LIBUSB_SUCCESS) {
            std::cerr << libusb_error_name(err) << " on register hotplug add" << std::endl;
        }
    }
}

int DeviceList::update() {
    int errorCode = libusb_init(&(_usb_context));
    if(errorCode != LIBUSB_SUCCESS)
        return errorCode;

    libusb_device **deviceList;

    ssize_t deviceCount = libusb_get_device_list(_usb_context, &deviceList);
    std::cout << "deviceList update() deviceCount: " << deviceCount << std::endl;
    if(deviceCount < 0)
        return LIBUSB_ERROR_NO_DEVICE;

    for(ssize_t deviceIterator = 0; deviceIterator < deviceCount; ++deviceIterator) {
        hotplugAdd(deviceList[deviceIterator]);
    }

    /// \todo Remove old devices

    libusb_free_device_list(deviceList, true);
    return errorCode;
}

void DeviceList::addDevice(DeviceBase* device) {
    _deviceList.push_back(std::shared_ptr<DeviceBase>(device));
    _listChanged();
}

std::shared_ptr<DeviceBase> DeviceList::getDeviceByUID(unsigned uid)
{
    for (std::shared_ptr<DSO::DeviceBase>& device: _deviceList) {
        std::cout << "getDeviceByUID: uid: " << device->getUniqueID() << std::endl;
        if (device->getUniqueID() == uid)
            return device;
    }
    return nullptr;
}

const std::vector<std::shared_ptr<DeviceBase> >& DeviceList::getList() const {
    return _deviceList;
}

const std::vector<DSODeviceDescription> DeviceList::getKnownModels() const
{
    std::cout << "getKnownModels()" << std::endl;
    return _registeredModels;
}

}
