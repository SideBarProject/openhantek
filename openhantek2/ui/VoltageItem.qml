import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.1

Grid {
//    property alias channelName: checkBox.text
    id: grid
    Layout.fillWidth: true;
    rows: 6
    flow: GridLayout.LeftToRight
    columnSpacing: 5
    rowSpacing: 5
/* channel A specs */
    CheckBox {
        id: checkBoxA;
        text: "Ch A";
        Layout.fillWidth: true;
    }
    ComboBox {
        id: voltageA;
        model:["5V","2.5V","1V","500mV"];
        Layout.fillWidth: true;
    }
    Label {text:"coupling: DC";
    }

    Label {text:" "; elide: Text.ElideRight; width: label_width}
/* channel B specs */
    Label {text:""}
    CheckBox {
        id: checkBoxB;
        text: "Ch B";
        Layout.fillWidth: true;
    }
    ComboBox {
        id: voltageB;
        model:["5V","2.5V","1V","500mV"];
        Layout.fillWidth: true;
    }
    Label {text:"coupling: DC";
    }
    Label {text:" "; elide: Text.ElideRight; width: label_width}
/* this one for maths */
    CheckBox {
        id: checkBoxM;
        text: "Maths";
        Layout.fillWidth: true;
    }
    ComboBox {
        id: math;
        Layout.fillWidth: true;
        model:["A+B","A-B","A*B","A/B"];

    }
}
