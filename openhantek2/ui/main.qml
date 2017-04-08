import QtQuick 2.4
import QtQuick.Controls 1.4
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
//import QtQuick.Extras 1.4 // A straight gauge; CircularGauge; Dial
import Qt.labs.settings 1.0

ApplicationWindow {
    id: window
    title: qsTr("OpenHantek")
    width: 640
    height: 480
    visible: true

    property string version: "0.0"

    signal qmlGetDevicesInfo()
    signal qmlSearchDevices(string deviceName)
    signal qmlInstallFW(string deviceName)

    Settings {
          property alias x: window.x
          property alias y: window.y
          property alias width: window.width
          property alias height: window.height
      }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&OpenHantek")
            MenuItem {
                text: qsTr("Select &Device")
                shortcut: "Ctrl+D"
                onTriggered: currentDevice.resetDevice()
                enabled: currentDevice.valid
            }
            MenuItem {
                text: qsTr("&Print")
                shortcut: "Ctrl+P"
                onTriggered: exportDialog.open();
                enabled: currentDevice.valid
            }
            MenuItem {
                text: qsTr("&Export as...")
                shortcut: "Ctrl+E"
                onTriggered: exportDialog.open();
                enabled: currentDevice.valid
            }
            MenuSeparator { }
            MenuItem {
                text: qsTr("E&xit")
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: qsTr("&Help")
            MenuItem {
                text: qsTr("Version: %1").arg(window.version)
                enabled: false
            }
            MenuSeparator { }
            MenuItem {
                text: qsTr("&Show website")
                shortcut: "Ctrl+W"
                onTriggered: Qt.openUrlExternally("http://openhantek.org");
            }
            MenuItem {
                text: qsTr("&Report an issue")
                shortcut: "Ctrl+R"
                onTriggered: Qt.openUrlExternally("http://github.com/openhantek/openhantek/issues");
            }
        }
    }
    MainForm {
        id: main
        objectName: "main"
        anchors.fill: parent
        visible: currentDevice.valid
    }

    DeviceList {
        id: deviceList
        width: 200
        height: 200
        anchors {
            centerIn: parent
        }
        visible: !currentDevice.valid
    }

    ExportDialog {
        id: exportDialog
    }
}
