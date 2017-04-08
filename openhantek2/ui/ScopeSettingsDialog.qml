import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import Qt.labs.settings 1.0
import QtQuick.Controls.Styles 1.4
import ModelDataSetter 1.0

Dialog {
    id: dialog
    standardButtons: StandardButton.Ok
    title: qsTr("Scope preferences")

    width: 300

    GridLayout {
        id: layout
        columns: 2
        rowSpacing: 10
        anchors.left: parent.left
        anchors.right: parent.right
        Layout.minimumWidth: 300

        Text {
            text: qsTr("Graph")
            Layout.columnSpan: 2
        }

        Label { Layout.leftMargin: 10; text: qsTr("Interpolation") }
        ComboBox {
            id: interpolationType
            Layout.fillWidth: true
            model: ListModel {
                ListElement { text: qsTr("Off"); type: "off" }
                ListElement { text: qsTr("Linear"); type: "linear" }
                ListElement { text: qsTr("Cubic"); type: "cubic" }
            }
        }

        Label { Layout.leftMargin: 10; text: qsTr("Motionblur") }
        ComboBox {
            id: motionblurType
            Layout.fillWidth: true
            model: ListModel {
                ListElement { text: qsTr("Off"); type: "0" }
                ListElement { text: qsTr("Soft"); type: "1" }
                ListElement { text: qsTr("Heavy"); type: "2" }
            }
        }

        Text {
            text: qsTr("Colors")
            Layout.columnSpan: 2
        }

        ListView {
            id: colors
            Layout.leftMargin: 10
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.minimumHeight: 100
            Layout.fillHeight: true
            model: screenColors
            interactive: true
            clip: true
            boundsBehavior: Flickable.StopAtBounds
            delegate: Component {
                Button {
                    id: edit
                    width: colors.width
                    //height: 20
                    //anchors.fill: parent
                    text: model.name
                    Layout.fillWidth: true
                    style: ButtonStyle {
                        label: Component {
                            Row {
                                spacing: 5
                                Rectangle {
                                    id: image
                                    color: model.color
                                    height: edit.height -5
                                    width: 16
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
                    }
                    onClicked: {
                        colorDialog.setIndex(model.index)
                        colorDialog.color = model.color
                        colorDialog.open()
                    }
                }
            }
        }
    }

    ColorDialog {
        id: colorDialog
        showAlphaChannel: false

        property int index

        function setIndex(index) {
            this.index = index;
        }

        onAccepted: {
            modelDataSetter.setModelData(colors.model, index, "color", currentColor)
        }

    }

    ModelDataSetter {
        id: modelDataSetter
    }

    Settings {
        property alias interpolationType: interpolationType.currentIndex
    }
}

