import QtQuick 2.0

Item {
    property real tickLength: 10
    property alias textcolor: label.color
    property alias tickcolor: tick.color
    property alias text: label.text
    property alias font: label.font

    property int tickAxis: Qt.YAxis

    implicitHeight: if (tickAxis === Qt.XAxis) {
                        tickLength + label.height + label.anchors.topMargin
                    } else if (tickAxis === Qt.YAxis) {
                        label.implicitHeight
                    }
    implicitWidth: if (tickAxis === Qt.XAxis) {
                       label.implicitWidth
                   } else if (tickAxis === Qt.YAxis) {
                       tickLength + label.width + label.anchors.rightMargin
                   }

    Rectangle {
        id: tick
        height: (tickAxis === Qt.XAxis) ? tickLength : 1
        width: (tickAxis === Qt.YAxis) ? tickLength : 1
        color: tick.color

        anchors {
            verticalCenter: (tickAxis === Qt.YAxis) ? label.verticalCenter : undefined
            right: (tickAxis === Qt.YAxis) ? parent.right : undefined

            horizontalCenter: (tickAxis === Qt.XAxis) ? label.horizontalCenter : undefined
            top : (tickAxis === Qt.XAxis) ? parent.top : undefined
        }
    }

    Text {
        id: label
        width: implicitWidth
        height: implicitHeight
        anchors {
            right: (tickAxis === Qt.YAxis) ? tick.left : undefined
            rightMargin: 5

            top: (tickAxis === Qt.XAxis) ? tick.bottom : undefined
            topMargin: 5
        }
    }
}
