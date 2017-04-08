import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Grid {
    property alias channelName: checkBox.text
    id: grid
    Layout.fillWidth: true;
    columns: 2
    flow: GridLayout.LeftToRight
    columnSpacing: 5
    rowSpacing: 5

    CheckBox { id: checkBox; Layout.fillWidth: true; width: label_width}
    ComboBox {
        model: [
            "1dB","2dB","3dB","6dB","10dB","20dB","30dB","60dB","100dB","200dB","300dB","600dB"
        ]
        id: spectrum;
        Layout.fillWidth: true; }
}
