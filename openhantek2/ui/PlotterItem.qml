import QtQuick 2.0
import QtQuick.Controls 1.3

Item {
// For digital phosphor effect
//    Rectangle {
//        id: rectangle_c
//        anchors.fill: parent
//        color: 'red'
//        visible: false
//    }

//    ShaderEffect {
//        anchors.fill: parent
//        property variant source: rectangle_c
//        property real frequency: 8
//        property real amplitude: 0.1
//        property real time: 0.0
//        NumberAnimation on time {
//            from: 0; to: Math.PI*2; duration: 1000; loops: Animation.Infinite
//        }

//        fragmentShader: "
//                        uniform lowp float qt_Opacity;
//                        void main() {
//                            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0) * qt_Opacity;
//                        }"

//        fragmentShader: "
//            varying highp vec2 qt_TexCoord0;
//            uniform sampler2D source;
//            uniform lowp float qt_Opacity;
//            uniform highp float frequency;
//            uniform highp float amplitude;
//            uniform highp float time;
//            void main() {
//                highp vec2 pulse = sin(time - frequency * qt_TexCoord0);
//                highp vec2 coord = qt_TexCoord0 + amplitude * vec2(pulse.x, -pulse.x);
//                gl_FragColor = texture2D(source, coord) * qt_Opacity;
//            }"
//    }
}
