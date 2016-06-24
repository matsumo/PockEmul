import QtQuick 2.2

Flickable {
    id: flickArea

    property alias fileName: notes.fileName

    anchors.fill: parent
    contentWidth: parent.width; contentHeight: notes.height
    flickableDirection: Flickable.VerticalFlick
    clip: true
    Text {
        id: notes
        property string fileName: ""
        width: parent.width
        wrapMode: TextEdit.Wrap
        textFormat: Text.RichText
        text: ""
        font {family: "Helvetica"; pointSize: 14 }
        Component.onCompleted: {
            text = main.getReleaseNotes(fileName);
        }
        onLinkActivated: Qt.openUrlExternally(link)
    }

}


