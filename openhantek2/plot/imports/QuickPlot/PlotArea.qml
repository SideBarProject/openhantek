import QtQuick 2.0
import QuickPlot 1.0

Item {
    width: 400
    height: 300

    property alias hasXTicks: xlabels.visible
    property alias hasYTicks: ylabels.visible
    property alias axes: axes

    property var xScaleEngine: TightScaleEngine {
        axis: Qt.XAxis
    }

    property var yScaleEngine : TightScaleEngine {
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

        anchors {
            top: parent.top
            left: parent.left
            bottom: xlabels.top
        }
    }

    AxisLabels {
        id: xlabels
        scaleEngine: xScaleEngine

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
