import QtQuick 2.0

Rectangle {
    id: container

    property alias text: label.text
    property alias font: label.font

    property bool expand: true;

    signal clicked

    width: expand ? Math.max(label.width + 20 , parent.width) : label.width + 20 ;
    height: label.height + 6
    border.color: root.buttonBorderColor
    radius: 5

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: { container.clicked() }
    }

    Text {
        id: label
        color: root.textButtonColor
        anchors.centerIn: parent
    }

    states: State {
        name: "pressed"
        when: mouseArea.pressed
        PropertyChanges { target: container; color: "#333333" }
    }
}
