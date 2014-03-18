import QtQuick 1.0

Item {
    id: container

    property string buttonName: "NOT SET"
    property string target: "NOT SET"
    property string text: ""
    property string fontName: "Helvetica"
    property int fontSize: 14
    property color fontColor: "black"

    property bool active: false
    property string bgImage: "gfx/imagebutton_icon.png"
    property string bgImagePressed: "gfx/imagebutton_icon_pressed.png"
    property string bgImageActive: bgImagePressed

    signal clicked(string target, string button)

    width: 50
    height: 50

    opacity: enabled ? 1.0 : 0.5

    Image {
        id: background
        source: bgImage;
        fillMode: "PreserveAspectFit"
        anchors.fill: parent
        smooth: true
    }

    Text {
        anchors {
            centerIn: background
            margins: 10
        }
        font {
            family: container.fontName
            pointSize: container.fontSize
        }
        text: container.text
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: container.clicked(container.target, container.buttonName);
    }

    states: [
        State {
            name: 'pressed'; when: mouseArea.pressed
            PropertyChanges { target: background; source: bgImagePressed; }
        },
        State {
            name: 'active'; when: container.active
            PropertyChanges { target: background; source: bgImageActive; }
        }
    ]


}
