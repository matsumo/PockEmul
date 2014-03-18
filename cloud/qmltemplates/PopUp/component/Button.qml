import QtQuick 1.0

Item {
    id: container

    property string buttonName: "NOT SET"
    property string text: "NOT SET"

    property string fontName: "Helvetica"
    property int fontSize: 14
    property color fontColor: "black"

    property bool active: false

    property string bgImage: 'gfx/button.png';
    property string bgImagePressed: 'gfx/button_pressed.png';
    property string bgImageActive: 'gfx/button_active.png';

    signal clicked(string button)

    width: 140
    height: 60
    opacity: enabled ? 1.0 : 0.5    

    BorderImage {
        id: background
        border { top: 11; bottom: 40; left: 38; right: 38; }
        source: bgImage
        width: parent.width
        height: parent.height
    }

    Text {
        id: buttonLabel
        text: parent.text
        wrapMode: Text.WordWrap

        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        font {
            family: container.fontName
            pointSize: container.fontSize
        }
        color: container.fontColor
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: container.clicked(buttonName);
    }

    states: [
        State {
            name: 'pressed'; when: mouseArea.pressed
            PropertyChanges { target: background; source: bgImagePressed; border { left: 38; top: 37; right: 38; bottom: 15 } }
        },
        State {
            name: 'active'; when: container.active
            PropertyChanges { target: background; source: bgImageActive; }
        }
    ]
}
