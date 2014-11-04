import QtQuick 2.0

Rectangle {
    id:menu
    x:5
//    z: -9999999
    property int iconsize: 48

    Image {
        id:newPocket
        source: "qrc:/core/pocket.png"
        width:parent.iconsize
        height:parent.iconsize
        y:12
        MouseArea {
            anchors.fill: parent
            onClicked: sendNewPocket()
        }
    }
    Image {
        id:newExt
        source: "qrc:/core/ext.png"
        width:parent.iconsize
        height:parent.iconsize
        anchors.top: newPocket.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: sendNewExt()
        }
    }
    Image {
        id:dev
        source: "qrc:/core/dev.png"
        width:parent.iconsize
        height:parent.iconsize
        anchors.top: newExt.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: sendDev()
        }
    }
    Image {
        id:save
        source: "qrc:/core/save.png"
        width:parent.iconsize
        height:parent.iconsize
        anchors.top: dev.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: sendSave()
        }
    }
    Image {
        id:load
        source: "qrc:/core/load.png"
        width:parent.iconsize
        height:parent.iconsize
        anchors.top: save.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: sendLoad()
        }
    }
    Image {
        id: cloudImag
        source: "qrc:/core/cloud.png"
        width:parent.iconsize
        height:parent.iconsize
        anchors.top: load.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: { cloudShow();
//                thecloud.visible = true;
//                scene.visible = false;
//                menu.visible = false;
            }
        }
    }
    Image {
        id:book
        source: "qrc:/core/bookcase.png"
        width:parent.iconsize
        height:parent.iconsize
        anchors.top: cloudImag.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: sendBook()
        }
    }
    Image {
        id:exit
        source: "qrc:/core/exit.png"
        width:parent.iconsize
        height:parent.iconsize
        anchors.top: book.bottom
        MouseArea {
            anchors.fill: parent
            onClicked: sendExit()
        }
    }
}

