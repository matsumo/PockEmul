import QtQuick 2.0

Item {

    Column {
        id: column1
        x: 42
        y: 8
        width: 513
        height: 320

        Text {
            id: text1
            text: qsTr("Text")
            anchors.fill: parent
            font.pixelSize: 12
        }
    }

    TextEdit {
        id: textEdit1
        x: 469
        y: 158
        width: 80
        height: 20
        text: qsTr("Text Edit")
        font.pixelSize: 12
    }

}
