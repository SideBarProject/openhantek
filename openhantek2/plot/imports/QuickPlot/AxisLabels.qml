import QtQuick 2.0

Rectangle {
    id: labels

    property real spacing: 10
    property int precision: 2
    property var scaleEngine
    property alias font: prototype.font

    width: prototype.implicitWidth
    height: prototype.implicitHeight

    color: "#00000000"

    Tick {
        id: prototype
        tickAxis: scaleEngine.axis
        text: (-100).toFixed(precision)
        visible: false
    }

    QtObject {
        id: internal

        property int numTicks: if (scaleEngine.axis === Qt.XAxis) {
                                   Math.max(Math.floor(width / (prototype.implicitWidth + spacing)) + 1, 2)
                               } else if (scaleEngine.axis === Qt.YAxis) {
                                   Math.max(Math.floor(height / (prototype.implicitHeight + spacing)) + 1, 2)
                               }

        property real tickDiff: niceNum((scaleEngine.max - scaleEngine.min) / (numTicks - 1))

        property real min: Math.floor(scaleEngine.min / tickDiff) * tickDiff;
        property real max: Math.ceil(scaleEngine.max / tickDiff) * tickDiff;

        /* Based on
         * Nice Numbers for Graph Labels
         * by Paul Heckbert
         * from "Graphics Gems", Academic Press, 1990
         */
        function niceNum(range) {
            var exponent = Math.floor(Math.log(range) / Math.LN10);     /** exponent of range */
            var fraction = range / Math.pow(10, exponent);    /** fractional part of range */
            var niceFraction;                                 /** nice, rounded fraction */

            if (fraction <= 1)        niceFraction = 1;
            else if (fraction <= 2)   niceFraction = 2;
            else if (fraction <= 2.5) niceFraction = 2.5;
            else if (fraction <= 5)   niceFraction = 5;
            else                      niceFraction = 10;

            return niceFraction * Math.pow(10, exponent);
        }
    }

    Repeater {
        id: rep
        model: internal.numTicks

        Tick {
            property real tickVal: internal.min + internal.tickDiff * index

            tickAxis: scaleEngine.axis

            font: prototype.font

            // passing scaleEngine.min and scaleEngine.max is not relevant for the actual return value of map(), but we still put it here to
            // trigger property binding re-evaluation on min/max change
            x: (tickAxis === Qt.XAxis) ?
                   scaleEngine.map(tickVal, Qt.rect(0, 0, labels.width, labels.height), scaleEngine.min, scaleEngine.max) - implicitWidth / 2 :
                   0
            y: (tickAxis === Qt.YAxis) ?
                   scaleEngine.map(tickVal, Qt.rect(0, 0, labels.width, labels.height), scaleEngine.min, scaleEngine.max) - implicitHeight / 2 :
                   0

            visible: if (tickAxis === Qt.XAxis) {
                        x >= (-implicitWidth / 2 - 0.1) && (x + implicitWidth / 2) <= (labels.width + 0.1)
                     } else if (tickAxis === Qt.YAxis) {
                        y >= (-implicitHeight / 2 - 0.1) && (y + implicitHeight / 2) <= (labels.height + 0.1)
                     }

            anchors {
                right: tickAxis === Qt.YAxis ? labels.right : undefined
                top: tickAxis === Qt.XAxis ? labels.top : undefined
            }

            text: tickVal.toFixed(precision)
        }
    }
}
