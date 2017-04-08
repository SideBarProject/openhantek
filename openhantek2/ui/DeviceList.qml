/**
  * Shows a deviceList, based on the c++ model "src/deviceModel.cpp" or
  * a help text, button to the github openhantek wiki and a list with all
  * supported device models.
  */
import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import ErrorStrings 1.0

// We have two Items (Rectangles) in this component that are shown if
// a) Devices have been found
// b) No devices have been found
Item {
    // Devices have been found. Contains a label and a listview with connected devices.
    Rectangle {
        visible: deviceModel.count>0
        border.color: "#e2dfdd"
        border.width: 1
        anchors.centerIn: parent
        color: "#f9f6f3"
        height: 260
        width: 250

        Label {
            id: connected_devices
            text: qsTr("Connected devices")
            anchors.margins: 5
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        ListView {
            id: devices
            anchors.margins: 5
            anchors.top: connected_devices.bottom
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            model: deviceModel
            interactive: true
            clip: true
            visible: deviceModel.count>0
            populate: Transition {
                NumberAnimation { properties: "x,y"; duration: 1000 }
            }
            boundsBehavior: Flickable.StopAtBounds
            // This list has buttons as items. Each button has a text with the model name
            // and an icon that is either red or green depending on the fact if a firmware
            // is necessary or not.
            delegate: Component {
                Button {
                    id: edit
                    width: devices.width
                    text: model.name
                    Layout.fillWidth: true
                    onClicked: {
                        // Either upload firmware
                        if (model.needFirmware)
                            uploadFirmwareDialog.show(model.uid, model.name)
                        else
                            currentDevice.setDevice(model.uid)
                    }
                    style: ButtonStyle {
                        label: Component {
                            Row {
                                spacing: 5
                                Rectangle {
                                    id: image
                                    color: model.needFirmware ? "red" : "green"
                                    height: edit.height -5
                                    width: 16
                                    radius: 10
                                    anchors.top: parent.top
                                    anchors.bottom: parent.bottom
                                }
                                Text {
                                    text: edit.text
                                    clip: true
                                    wrapMode: Text.WordWrap
                                    verticalAlignment: Text.AlignVCenter
                                    horizontalAlignment: Text.AlignLeft
                                }
                            }
                        }
                    } // end style
                } // end button
            } // end delegate
        }
    }

    // No device found. Contains a label and a listview with supported models.
    Rectangle {
        visible: deviceModel.count==0
        border.color: "#e2dfdd"
        border.width: 1
        anchors.centerIn: parent
        color: "#f9f6f3"
        height: 260
        width: 250

        Label {
            id: no_devices_found
            anchors.margins: 5
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
            text: qsTr("No devices found. We support the following models")
        }

        RowLayout {
            id: supportedDevicesContainer
            anchors.margins: 5
            anchors.top: no_devices_found.bottom
            anchors.bottom: btnDemoMode.top
            anchors.left: parent.left
            anchors.right: parent.right
            ListView {
                id: supportedDevices
                anchors.fill: parent
                interactive: true
                clip: true
                model: deviceModel.supportedDevices
                delegate: Component {
                    Text {
                        text: model.modelData
                        Layout.fillWidth: true
                        width: supportedDevices.width
                    }
                }
            }
            Scrollbar {
                flk: supportedDevices
            }
        }

        Button {
            id: btnDemoMode
            anchors.margins: 5
            anchors.bottom: btnConnectionIssues.top
            anchors.left: parent.left
            anchors.right: parent.right
            onClicked: currentDevice.setDemoDevice()
            text: qsTr("Demo Mode (Sine Wave)")
        }

        Button {
            id: btnConnectionIssues
            anchors.margins: 5
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            onClicked: Qt.openUrlExternally("http://github.com/openhantek/openhantek/wiki");
            text: qsTr("Advices on connection issues")
        }
    }



    MessageDialog {
        id: uploadFirmwareDialog
        standardButtons: StandardButton.Yes | StandardButton.No
        title: qsTr("This device needs firmware")
        property int uid
        function show(uid, name) {
            this.uid = uid
            uploadFirmwareDialog.text = qsTr("Upload firmware for %1?").arg(name);
            uploadFirmwareDialog.open();
        }
        onYes: {
            var r = deviceModel.uploadFirmware(uid);
            if (r != 0) {
                uploadFirmwareFailedDialog.show(r);
            }
        }
    }

    MessageDialog {
        id: uploadFirmwareFailedDialog
        standardButtons: StandardButton.Ok
        title: qsTr("Uploading firmware failed")
        function show(error_code) {
            uploadFirmwareFailedDialog.text = errorStrings.errorString(error_code);
            uploadFirmwareFailedDialog.open();
        }
    }

    ErrorStrings {
        id: errorStrings
    }

}

