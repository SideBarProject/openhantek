import QtQuick 2.1
import QuickPlot 1.0

Rectangle {
    visible: true
    width: 640
    height: 480

    MouseArea {
        anchors.fill: parent
        onClicked: {
            Qt.quit();
        }
    }

    PlotArea {
        id: plotArea
        anchors.fill: parent

        yScaleEngine: FixedScaleEngine {
            max: 1.5
            min: -1.5
        }

        items: [
            ScrollingCurve {
                id: meter;
                numPoints: 300
            }
        ]
    }

    Timer {
        id: timer;
        interval: 20;
        repeat: true;
        running: true;

        property real pos: 0

        onTriggered: {
            meter.appendDataPoint( Math.sin(pos) );
            pos += 0.05;
        }
    }
}
