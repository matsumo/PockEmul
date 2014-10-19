/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
import QtQuick 2.3
import QtQuick.XmlListModel 2.0
import QtQuick.Window 2.1
import QtQuick.Controls 1.2



Rectangle {
    id: renderArea

    signal sendWarning(string test)
    signal sendKeyPressed(string id,int key, int mod,int scancode)
    signal sendKeyReleased(string id,int key, int mod,int scancode)
    signal sendContextMenu(string id,int x,int y)
    signal sendClick(string id,int x,int y)
    signal sendUnClick(string id,int x,int y)
    signal sendMovePocket(string id,int x,int y)
    signal sendMoveAllPocket(int x,int y)
    signal setZoom(int x,int y,int z)

    signal sendNewPocket();
    signal sendNewExt();
    signal sendDev();
    signal sendSave();
    signal sendLoad();
    signal sendBook();
    signal sendExit();

     property alias xmlThumbModel: xmlThumbModel

    visible: true
    width: 1024; height: 600
//    color: "black"

    property int highestZ: 0
    property real defaultSize: 200
    property int prevX: 0
    property int prevY: 0



Rectangle {
    id: scene
    anchors.fill: parent

    PinchArea {
        id: mainpinch
        anchors.fill: parent
        //pinch.target: photoFrame
        pinch.minimumRotation: -360
        pinch.maximumRotation: 360
        pinch.minimumScale: 0.1
        pinch.maximumScale: 10
        onPinchUpdated: {
            setZoom(pinch.startCenter.x,pinch.startCenter.y,pinch.scale*100 - 100);
        }
        MouseArea {
            property bool isdrag: false;
            hoverEnabled: false
            anchors.fill: parent
            onWheel: {
//                console.log("wheel:"+wheel.x+wheel.y+wheel.angleDelta);
                    setZoom(wheel.x,wheel.y,wheel.angleDelta.y/12);
            }
            onPressed: {
                prevX = mouseX;
                prevY = mouseY;
                isdrag=true;
            }
            onReleased: isdrag=false;
            onPositionChanged: {
                if (isdrag) {
                    console.log("master isdrag");
                    sendMoveAllPocket(mouseX-prevX,mouseY-prevY);
                    prevX = mouseX;
                    prevY = mouseY;
                }
            }
        }
    }

    ListModel {
        id: xmlThumbModel
    }


    Repeater {
        id: repeater
        model: xmlThumbModel

        Rectangle {
            id: photoFrame
            color: "transparent"

            border.width: 0
//            smooth: true
//            antialiasing: true
            x: _left
            y: _top
            z: _zorder
            width: _width
            height: _height
//            rotation: Math.random() * 13 - 6
            Keys.onPressed: {
                sendKeyPressed(idpocket,event.key,event.modifiers,event.nativeScanCode);
                event.accepted = true;
                }
            Keys.onReleased: {
                sendKeyReleased(idpocket,event.key,event.modifiers,event.nativeScanCode);
                event.accepted = true;
                }
            Image {
                id: image
                anchors.fill: parent
                fillMode: Image.Stretch
                source: "image://Pocket/"+idpocket+"/"+dummy
                scale: 1
                smooth: true
                mipmap: true
                antialiasing: true
            }
            PinchArea {
                anchors.fill: parent
                pinch.target: photoFrame
                pinch.minimumRotation: -360
                pinch.maximumRotation: 360
                pinch.minimumScale: 0.1
                pinch.maximumScale: 10
                onPinchUpdated: {
                    setZoom(pinch.startCenter.x,pinch.startCenter.y,pinch.scale*100 - 100);
                }
                MouseArea {
                    property bool isdrag: false;

                    id: dragArea
                    hoverEnabled: false;
//                    preventStealing:true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    propagateComposedEvents: false
                    drag.target: photoFrame
                    onPressAndHold: {
                        if (mouse.button == Qt.LeftButton) {
                            sendContextMenu(idpocket,mouse.x,mouse.y);
                        }
                        isdrag=false;
                    }
                    onPressed: {
                        drag.maximumX = 65536; //renderArea.width;
                        drag.minimumX = -65536; //0;
                        drag.maximumY = 65536; //renderArea.height;
                        drag.minimumY = -65536; //0;
                        photoFrame.focus = true;
                        if (mouse.button == Qt.RightButton) {
//                            console.log("drag active:"+drag.active);
                            sendContextMenu(idpocket,mouse.x,mouse.y);
                            isdrag=false;
                        }
                        if (mouse.button == Qt.LeftButton) {
                            isdrag=true;
//                            console.log("isdrag true");
                            sendClick(idpocket,mouse.x,mouse.y);
                        }
                        mouse.accepted=true;
                    }
                    onReleased: {
                        isdrag=false;
                        sendUnClick(idpocket,mouseX,mouseY);
                    }
                    onPositionChanged: {
//                        console.log("move isdrag active:"+drag.active+" isdrag:"+isdrag);
                        if (isdrag) {
                            sendMovePocket(idpocket,photoFrame.x,photoFrame.y);
//                          console.log("diff:" +(_tmpX-prevX) +" "+(_tmpY-prevY));
                        }
                        else {
//                            console.log("drag cancel");
                            drag.maximumX = photoFrame.x;
                            drag.minimumX = photoFrame.x;
                            drag.maximumY = photoFrame.y;
                            drag.minimumY = photoFrame.y;
                        }

                    }
//                    onEntered: photoFrame.border.color = "red";
//                    onExited: photoFrame.border.color = "black";
                    onWheel: {
                        wheel.accepted = false;
                        if (wheel.modifiers & Qt.ControlModifier) {
                            wheel.accepted = true;
                            photoFrame.rotation += wheel.angleDelta.y / 120 * 5;
                            if (Math.abs(photoFrame.rotation) < 4)
                                photoFrame.rotation = 0;
                        }
                    }
                }
            }
        }
    }
}


Rectangle {
    id:menu
    x:5
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
            onClicked: {
                thecloud.visible = true;
                scene.visible = false;
                menu.visible = false;
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
    Main {
        id: thecloud
        anchors.fill: parent
        visible: false;
    }
    Component.onCompleted: {
        thecloud.close.connect(test)
    }
    function test() {
        console.log("close");
        thecloud.visible = false;
        scene.visible = true;
        menu.visible = true;
    }

    function addPocket(_name,_url,_pocketId,_left,_top,_width,_height) {
        renderArea.xmlThumbModel.append(   {name:_name,
                                 imageFileName:_url,
                                 _left:_left,
                                 _top:_top,
                                 _width:_width,
                                 _height:_height,
                                 idpocket:_pocketId,
                                      dummy:0,
                                      _zorder:0});

    }

    function refreshPocket(_pocketId) {
        var index = getIndex(_pocketId);
        if (index == -1) return;
        renderArea.xmlThumbModel.get(index).dummy = Math.random();
    }

    function delPocket(_pocketId) {
        var index = getIndex(_pocketId);
        renderArea.xmlThumbModel.remove(index);
    }

    function movePocket(_pocketId,_left,_top) {

        var index = getIndex(_pocketId);

    //    console.log("found index:"+index);
        if (index !== -1) {

//            console.log("object moved from ("+renderArea.xmlThumbModel.get(index)._left+","+renderArea.xmlThumbModel.get(index)._top+") to ("+_left+","+_top+")");
            renderArea.xmlThumbModel.get(index)._left = _left;
            renderArea.xmlThumbModel.get(index)._top = _top;
        }
    }

    function sizePocket(_pocketId,_width,_height) {

        var index = getIndex(_pocketId);

    //    console.log("found index:"+index);
        if (index !== -1) {

            renderArea.xmlThumbModel.get(index)._width = _width;
            renderArea.xmlThumbModel.get(index)._height = _height;
    //        console.log("object sized to ("+_width+","+_height+")");
        }
    }

    function orderPocket(_pocketId,_zorder) {

        var index = getIndex(_pocketId);

    //    console.log("found index:"+index);
        if (index !== -1) {

            renderArea.xmlThumbModel.get(index)._zorder = _zorder;
        }
    }

    function getIndex(id) {
        for (var i=0; i<renderArea.xmlThumbModel.count;i++) {
            var item = renderArea.xmlThumbModel.get(i);
            if (item.idpocket === id) {
                return i;
            }
        }
        return -1;
    }


}
