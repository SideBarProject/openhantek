#pragma once

#include <vector>
#include <memory>

#include "deviceBase.h"

class libusb_context;
class libusb_device;

namespace DSO {

/**
 * A list of all connected usb DSO devices.
 */
class DeviceList {
    public:
        DeviceList();
        ~DeviceList();

        /// Register a model. This will be considered on the next usb plug event if
        /// autoupdate is enabled or if update() is called.
        void registerModel(const DSODeviceDescription& model);

        /// Enable/disable automatic device discovery / hotplugging
        void setAutoUpdate(bool autoUpdate);

        /// Update the deviceList manually
        /// \return Return a libusb errorcode or LIBUSB_SUCCESS if successful.
        int update();

        /*
         * Add device to the list. You need to set the keepInList flag to the
         * device object if you want it to survive an automatic update if
         * automatic updates are enabled.
         *
         * This will take over the pointer and destroy the object if the device
         * is removed from the list.
         */
        void addDevice(DeviceBase* device);

        /**
          * Return the device with the given device unique id.
          */
        std::shared_ptr<DeviceBase> getDeviceByUID(unsigned uid);

        /// Get list of devices. This is a vector of unique pointers
        /// -> You are not allowed to copy this vector.
        const std::vector<std::shared_ptr<DeviceBase>>& getList() const;

        /// Get list of known models.
        const std::vector<DSODeviceDescription> getKnownModels() const;

        void hotplugAdd(libusb_device *device);
        void hotplugRemove(libusb_device *device);

        /// To be called periodically so that hotplug events work
        void checkForDevices() const;

        /// Signal: list has changed
        std::function<void(void)> _listChanged = [](){};
        /// Signal: supported models changed
        std::function<void(void)> _modelsChanged = [](){};
private:
    std::vector<std::shared_ptr<DeviceBase>> _deviceList;
    std::vector<DSODeviceDescription> _registeredModels;
    bool _autoUpdate = false;
    libusb_context* _usb_context = nullptr;
    int _callback_handle = 0;
};

}
