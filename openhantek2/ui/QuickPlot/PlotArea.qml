import QtQuick 2.0
import QuickPlot 1.0

Rectangle {
    width: 400
    height: 300

    property alias hasXTicks: xlabels.visible
    property alias hasYTicks: ylabels.visible
    property alias axes: axes

    color: "black"
    property color borderColor: "white"
    property color gridColor: "white"
    property color markersColor: "white"
    property color textColor: "white"
    property color axesColor: Qt.rgba(0.5, 0.5, 0.5, 0.5)

    property var xScaleEngine: ScaleEngine {
        id: xScale
        axis: Qt.XAxis
    }

    property var yScaleEngine : ScaleEngine {
        axis: Qt.YAxis
    }

    onXScaleEngineChanged: {
        xScaleEngine.axis = Qt.XAxis;
        this._updateDependencies();
    }
    onYScaleEngineChanged: {
        yScaleEngine.axis = Qt.YAxis;
        this._updateDependencies();
    }

    AxisLabels {
        id: ylabels
        scaleEngine: yScaleEngine
        textcolor: parent.textColor
        tickcolor: axesColor
        anchors {
            top: parent.top
            left: parent.left
            bottom: xlabels.top
        }
    }

    AxisLabels {
        id: xlabels
        scaleEngine: xScaleEngine
        textcolor: parent.textColor
        tickcolor: axesColor
        anchors {
            left: ylabels.right
            right: parent.right
            bottom: parent.bottom
        }
    }

    Axes {
        id: axes
        xScaleEngine: parent.xScaleEngine
        yScaleEngine: parent.yScaleEngine
        borderColor: parent.borderColor
        zeroLineColor: parent.axesColor
        anchors {
            left: ylabels.right;
            right: parent.right;
            top: parent.top;
            bottom: xlabels.top;
        }
    }

    property list<PlotItem> items

    Item {
        clip: true
        Item {
            id: plotItemRescaler
            anchors.fill: parent

            property rect boundingRect: Qt.rect(0, 0, width, height)

            transform: [
                Translate {
                    x: -xScaleEngine.min
                    y: -yScaleEngine.min
                },

                Scale {
                    xScale: xScaleEngine.scaleFactor(plotItemRescaler.boundingRect,
                                                     xScaleEngine.min, xScaleEngine.max);
                    yScale: yScaleEngine.scaleFactor(plotItemRescaler.boundingRect,
                                                     yScaleEngine.min, yScaleEngine.max);
                },

                Translate {
                    x: 0
                    y: plotItemRescaler.height
                }
            ]
        }

        anchors {
            left: ylabels.right
            right: parent.right
            top: parent.top
            bottom: xlabels.top
        }
    }

    Component.onCompleted: {
        this._updateDependencies();
        onItemsChanged.connect(this._updateDependencies);
    }

    function _updateDependencies() {
        for (var i = 0; i < items.length; ++i) {
            items[i].parent = plotItemRescaler;
            items[i].anchors.fill = plotItemRescaler;
        }
        xScaleEngine.plotItems = items;
        yScaleEngine.plotItems = items;
    }
}
