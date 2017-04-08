import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4

ColumnLayout {
    id: item
    anchors.left: parent.left
    anchors.right: parent.right
    clip: true

    property alias title: btnHeader.text
    property string collapseImage: "images/collapse.png"
    property string expandImage: "images/expand.png"
//    property string collapseImage: "qrc:/collapse.png"
//    property string expandImage: "qrc:/expand.png"
    property bool contentVisible: true
    property bool hasSettings: false
    property alias btnPreferences: btnPreferences

    Layout.preferredHeight: implicitHeight
    states: State {
        name: "only_header"
        when: !contentVisible
        PropertyChanges { target: item; Layout.preferredHeight: btnRow.height }
    }

    Behavior on Layout.preferredHeight {
        NumberAnimation {
            id: bouncebehavior
            duration: 300
            easing {
                type: Easing.OutCubic
                amplitude: 1.0
                period: 0.5
            }
        }
    }

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        id: btnRow

        Button {
            id: btnHeader
            text: "default header"
            Layout.fillWidth: true
            style: ButtonStyle {
                label: Component {
                    Row {
                        spacing: 5
                        Image {
                            id: image
                            source: btnHeader.iconSource
                        }
                        Text {
                            text: btnHeader.text
                            clip: true
                            wrapMode: Text.WordWrap
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignLeft
                        }
                    }
                }
            }

            iconSource: contentVisible ? collapseImage : expandImage
            onClicked: { contentVisible = !contentVisible }
        }

        Button {
            id: btnPreferences
            iconSource: "images/preferences.png"
            visible: item.hasSettings
            height: btnHeader.height
        }
    }
}

