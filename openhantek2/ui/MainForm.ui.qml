import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1
import Qt.labs.settings 1.0
import "QuickPlot"
import QuickPlot 1.0

Item {
    id: page
    width: 640
    height: 480
    Layout.minimumWidth: 500
    Layout.minimumHeight: 300
    anchors.margins: 10

    property int label_width: 100

    Flickable {
        id: flick
        //clip: true;
        contentWidth: width;
        contentHeight: allControls.height
        Layout.minimumWidth: 200
        Layout.maximumWidth: 400
        width: 250
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }

        ColumnLayout {
        id: allControls
        anchors {
            left: parent.left
            right: parent.right
        }

        ExpandableItem {
            id: groupHorizontal
            title: qsTr("Horizontal")
            Layout.fillWidth: true;

            Grid {
                Layout.fillWidth: true;
                columns: 2
                flow: GridLayout.LeftToRight
                columnSpacing: 5
                rowSpacing: 5

                Label { text: qsTr("Time/Div"); elide: Text.ElideRight; width: label_width }
                ComboBox { id: comboBox1;
                    model: [
                        "100ns","200ns","500ns",
                        "1\u00b5s","2\u00b5s","5\u00b5s",
                        "10\u00b5s","20\u00b5s","50\u00b5s",
                        "100\u00b5s","200\u00b5s","500\u00b5s",
                        "1ms","2ms","5ms",
                        "10ms","20ms","50ms",
                        "100ms","200ms","500ms"
                    ];
                    Layout.fillWidth: true;
                }

                Label { text: qsTr("Format"); elide: Text.ElideRight; width: label_width }
                ComboBox {
                    id: comboBox2;
                    model: ["Y-T","X-Y"];
                    Layout.fillWidth: true;
                }
            }
        }

        ExpandableItem {
            id: groupAdvanced
            contentVisible: false
            Layout.fillWidth: true;
            title: qsTr("Advanced")

            Grid {
                Layout.fillWidth: true;
                columns: 2
                flow: GridLayout.LeftToRight
                columnSpacing: 5
                rowSpacing: 5

                Label { text: qsTr("Samplerate"); elide: Text.ElideRight; width: label_width }
                ComboBox {
                    id: comboBox1b;
                    Layout.fillWidth: true;
                    model: ["48MHz","30MHz","24MHz","16MHz","8MHz","4MHz","1MHz","500kHz","200kHz","100kHz"];
                }

                Label { text: qsTr("Record length"); elide: Text.ElideRight; width: label_width }
                ComboBox { id: comboBox2b; Layout.fillWidth: true; }
            }
        }

        ExpandableItem {
            id: groupTrigger
            title: qsTr("Trigger")

            Grid {
                Layout.fillWidth: true;
                columns: 2
                flow: GridLayout.TopToBottom
                columnSpacing: 5
                rowSpacing: 5

                Label { text: qsTr("Mode"); elide: Text.ElideRight; width: label_width }
                Label { text: qsTr("Source"); elide: Text.ElideRight; width: label_width }
                Label { text: qsTr("Slope"); elide: Text.ElideRight; width: label_width }

                ComboBox {
                    id: triggerModeComboBox;
                    Layout.fillWidth: true;
                    model: ["Auto","Normal","Single Shot"];
                }
                ComboBox {
                    id: comboBox2a;
                    Layout.fillWidth: true;
                    model: ["Channel A","Channel B"];
                }
                ComboBox {
                    id: comboBox3a;
                    Layout.fillWidth: true;
                    model:["\u2197","\u2198"];
                }
            }
        }

        ExpandableItem {
            id: groupVoltage
            title: qsTr("Voltage")

            VoltageItem {
                id: voltageItem
//                channelName: qsTr("Ch A")
            }

        }

        ExpandableItem {
            id: groupSpectrum
            contentVisible: false
            hasSettings: true
            btnPreferences.onClicked: spectrumSettingsDialog.open()
            title: qsTr("Spectrum")

            SpectrumItem {
                channelName: qsTr("Ch A")
            }
            SpectrumItem {
                channelName: qsTr("Ch B")
            }
            SpectrumItem {
                channelName: qsTr("Math")
            }
        }

        } // end column
    }

    Settings {
        property alias groupHorizontal: groupHorizontal.contentVisible
        property alias groupAdvanced: groupAdvanced.contentVisible
        property alias groupTrigger: groupTrigger.contentVisible
        property alias groupSpectrum: groupSpectrum.contentVisible
        property alias groupVoltage: groupVoltage.contentVisible
    }

    Item {
        id: plot
        anchors.leftMargin: 10
        Layout.minimumWidth: 200
        anchors {
            left: flick.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }

        PlotArea {
            id: plotarea
            anchors.fill: parent
            color: screenColors.background
            borderColor: screenColors.border
            gridColor: screenColors.grid
            markersColor: screenColors.markers
            textColor: screenColors.text
            axesColor: screenColors.axes

            yScaleEngine: currentDevice.yScaleEngine
            xScaleEngine: currentDevice.xScaleEngine
            items: currentDevice.channels
        }

        Button {
            id: btnPreferences
//            iconSource: "qrc:preferences.png"
            iconSource: "images/preferences.png"
            width: 32
            height: 32
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 10
            anchors.rightMargin: 10
            onClicked: scopeSettingsDialog.open()
        }
    }

    ScopeSettingsDialog {
        id: scopeSettingsDialog
    }

    SpectrumSettingsDialog {
        id: spectrumSettingsDialog
    }
}
