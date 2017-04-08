import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0

Dialog {
    id: dialog
    standardButtons: StandardButton.Ok
    title: qsTr("Spectrum preferences")

    width: 300

    GridLayout {
        id: layout
        columns: 2
        rowSpacing: 10
        anchors.left: parent.left
        anchors.right: parent.right
        Layout.minimumWidth: 300

        Label { Layout.leftMargin: 10; text: qsTr("Window function") }
        ComboBox {
            id: windowFunctionType
            model: ListModel {
                ListElement { text: qsTr("Rectangular");}
                ListElement { text: qsTr("Hamming");}
                ListElement { text: qsTr("Hann");}
                ListElement { text: qsTr("Cosine");}
                ListElement { text: qsTr("Lanczos");}
                ListElement { text: qsTr("Bartlett");}
                ListElement { text: qsTr("Triangular");}
                ListElement { text: qsTr("Gauss");}
                ListElement { text: qsTr("Bartlett-Hann");}
                ListElement { text: qsTr("Blackman");}
                ListElement { text: qsTr("Nuttall");}
                ListElement { text: qsTr("Blackman-Harris");}
                ListElement { text: qsTr("Blackman-Nuttall");}
                ListElement { text: qsTr("Flat top");}
            }
        }

        Label { Layout.leftMargin: 10; text: qsTr("Reference level") }
        SpinBox {
            id:  referenceLevel
            value: 0.0
            decimals: 1
            minimumValue: -40.0
            maximumValue: 100.0
            suffix: qsTr("dbm")
        }
        Label { Layout.leftMargin: 10; text: qsTr("Minimum magnitude") }
        SpinBox {
            id:  minimumMagnitude
            value: 0.0
            decimals: 1
            minimumValue: -40.0
            maximumValue: 100.0
            suffix: qsTr("dbm")
        }
    }

    Settings {
        property alias windowFunctionType: windowFunctionType.currentIndex
        property alias referenceLevel: referenceLevel.value
        property alias minimumMagnitude: minimumMagnitude.value
    }
}

