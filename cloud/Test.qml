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
import "content"
import "."


Rectangle {
    id: renderArea

    signal sendWarning(string test)
    signal sendKeyPressed(string id,int key, int mod,int scancode)
    signal sendKeyReleased(string id,int key, int mod,int scancode)
    signal sendContextMenu(string id,int x,int y)
    signal sendClick(string id,int x,int y)
    signal sendUnClick(string id,int x,int y)
    signal sendDblClick(string id,int x,int y)
    signal sendMovePocket(string id,int x,int y)
    signal sendMoveAllPocket(int x,int y)
    signal setZoom(int x,int y,real z)
    signal sendRotPocket(string id,int rotation)
    signal maximize(string id)
    signal minimize(string id)

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
    color: "black" //transparent"

    property int highestZ: 0
    property real defaultSize: 200
    property int prevX: 0
    property int prevY: 0



    Rectangle {
        id: scene
        anchors.fill: parent
        color: "transparent"

        PinchArea {
            id: mainpinch
            z: -9999
            property real previousScale: 1
            anchors.fill: parent
            pinch.minimumRotation: -360
            pinch.maximumRotation: 360
            pinch.minimumScale: 0.1
            pinch.maximumScale: 10
            onPinchUpdated: {
                console.warn("pinch master");
                setZoom(pinch.center.x,pinch.center.y,pinch.scale/pinch.previousScale);
                previousScale=pinch.scale;
            }

            MouseArea {
                property bool isdrag: false;
                hoverEnabled: false
                anchors.fill: parent
                onWheel: {
                    //                console.log("wheel:"+wheel.x+wheel.y+wheel.angleDelta);
                    console.warn("wheel");
                    setZoom(wheel.x,wheel.y,wheel.angleDelta.y/12>0 ? 1.1 : .9);
                }
                onPressed: {
                    //                console.warn("pressed MASTER");
                    prevX = mouseX;
                    prevY = mouseY;
                    this.isdrag=true;
                }
                onReleased: {
                    //                console.warn("release MASTER");
                    isdrag=false;
                }
                onDoubleClicked: {
                    for (var i=0; i<repeater.count;i++) {
                        repeater.itemAt(i).touchEnabled = !repeater.itemAt(i).touchEnabled;
                    }
                }
                onPositionChanged: {

                    if (isdrag) {
                        sendMoveAllPocket(mouseX-prevX,mouseY-prevY);
                        //                    console.warn("move MASTER:",mouseX-prevX,mouseY-prevY);
                        prevX = mouseX;
                        prevY = mouseY;
                    }
                }
            }   // end MouseArea
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

                property bool touchEnabled : true

                border.width: 0
                //            smooth: true
                //            antialiasing: true
                x: _left
                y: _top
                z: _zorder
                width: _width
                height: _height
                visible: _visible
                rotation: _rotation

                onRotationChanged: sendRotPocket(idpocket,rotation)

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

                //            PinchArea {
                //                property real previousScale: 1
                //                enabled: false
                //                anchors.fill: parent
                //                pinch.target: photoFrame
                //                pinch.minimumRotation: -360
                //                pinch.maximumRotation: 360
                //                pinch.minimumScale: 0.1
                //                pinch.maximumScale: 10
                //                onPinchStarted: previousScale=1;
                //                onPinchUpdated: {
                //                    console.warn("pinch");
                //                    setZoom(pinch.startCenter.x,pinch.startCenter.y,pinch.scale);
                //                    previousScale=pinch.scale;
                ////                    photoFrame.rotation = pinch.rotation
                //                }
                //            }

                MultiPointTouchArea {
                    id: pocketTouchArea
                    enabled: parent.touchEnabled
                    mouseEnabled: true;
                    anchors.fill: parent
                    //                minimumTouchPoints: 1
                    //                maximumTouchPoints: 2
                    //                touchPoints: [
                    //                    TouchPoint { id: point1 },
                    //                    TouchPoint { id: point2 }
                    //                ]
                    onPressed: {
                        for (var touch in touchPoints) {
                            //                        console.warn("Multitouch pressed touch", touchPoints[touch].pointId, "at", touchPoints[touch].x, ",", touchPoints[touch].y)
                            sendClick(idpocket,touchPoints[touch].x,touchPoints[touch].y);
                        }
                    }
                    onReleased: {

                        //                    console.warn("touchPoints count:",touchPoints.length);
                        for (var touch in touchPoints) {
                            //                        console.warn("Multitouch released touch", touchPoints[touch].pointId, "at", touchPoints[touch].x, ",", touchPoints[touch].y)
                            var tx = touchPoints[touch].x;
                            var ty = touchPoints[touch].y;
                            sendUnClick(idpocket,tx,ty);
                        }

                    }



                    MouseArea {
                        property bool isdrag: false;
                        property point previousPosition: Qt.point(1, 1);

                        id: dragArea
                        enabled: true;
                        hoverEnabled: true;
                        preventStealing: true
                        acceptedButtons: Qt.LeftButton | Qt.RightButton
                        anchors.fill: parent
                        propagateComposedEvents: false

//                        onPressAndHold: {
//                            isdrag=false;
//                            if (!main.keyAt(idpocket,mouse.x,mouse.y) && (mouse.button === Qt.LeftButton)) {
//                                sendContextMenu(idpocket,mouse.x,mouse.y);
//                            }
//                        }

                        onPressed: {
                            //                        console.warn("clik");
                            previousPosition = Qt.point(mouse.x, mouse.y);
                            photoFrame.focus = true;
                            if (mouse.button === Qt.RightButton) {
                                sendContextMenu(idpocket,mouse.x,mouse.y);
                                isdrag=false;
                            }
                            if (mouse.button === Qt.LeftButton) {
                                if (!main.keyAt(idpocket,mouse.x,mouse.y)) {
                                    isdrag=true;
                                }
                                sendClick(idpocket,mouse.x,mouse.y);
                            }
                            mouse.accepted=true;
                        }

                        onReleased: {
                            isdrag=false;
                            sendUnClick(idpocket,mouseX,mouseY);
                            mouse.accepted=true;

                        }

                        onDoubleClicked: {
                            isdrag = false;
                            if (main.keyAt(idpocket,mouse.x,mouse.y)) {
                                sendDblClick(idpocket,mouseX,mouseY);
                            }
                            else {
                                contextMenu.idpocket = idpocket;
                                contextMenu.width = Math.max(300,image.width/2);
                                contextMenu.mousePt = Qt.point(mouse.x,mouse.y);
                                contextMenu.buttons = mouse.buttons;
                                contextMenu.popup(photoFrame.x+mouseX, photoFrame.y+mouseY);
                                console.log("popup:",contextMenu.selectedOption);
                            }
                        }

                        onPositionChanged: {
                            if (isdrag) {
                                var dx = mouse.x - previousPosition.x
                                var dy = mouse.y - previousPosition.y
                                sendMovePocket(idpocket,photoFrame.x+dx,photoFrame.y+dy);
                            }
                            else {
                                if (main.keyAt(idpocket,mouse.x,mouse.y)) {
                                    cursorShape = Qt.PointingHandCursor;
                                }
                                else {
                                    cursorShape = Qt.ArrowCursor;
                                }
                            }
                            mouse.accepted=true;

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

        Launchmenu {
            id:menu
            z: mainpinch.z+1
        }

    }

    Main {
        id: thecloud
        anchors.fill: parent
        visible: false;
    }

    ContextMenu {
        id: contextMenu
    }

    Rectangle {
        id: workingScreen
        visible: false;
        anchors.fill: parent
        z: -9999
        color: "black"
        Row {
            anchors.centerIn: parent
            Column {
                spacing: 30
                BusyIndicator {
                    id: working
                    on: true
                    width: 200
                    height: 200
                }

                TextButton {
                    text: "Cancel"
                    expand: true
                    font.pointSize: 16
                    onClicked: {
                        if (cloud.askDialog("Do you really want to cancel ?",2)==2) return;
                        hideWorkingScreen();
                    }
                }
            }
        }
    }

    function showWorkingScreen() {
        console.log("show busy");
        workingScreen.z=9999;
        workingScreen.visible=true;
    }
    function hideWorkingScreen() {
        workingScreen.z=-9999;
        workingScreen.visible=false;
    }

    function manageContextResponse(option,idpocket, mousePt, buttons) {
        console.log("option:",option);
        if (option==="Zoom In") {
            // zoom
            maximize(idpocket);

        }
        else if (option==="Zoom Out") {
            // zoom
            minimize(idpocket);
        }
        else if (option==="Menu") {
            sendContextMenu(idpocket,mousePt.x,mousePt.y);
        }
        else if (option==="Pinch") {
            var ind = getIndex(idpocket);
//            console.log("pinch:",idpocket,ind);
            repeater.itemAt(ind).touchEnabled = !repeater.itemAt(ind).touchEnabled;

        }

    }

    Component.onCompleted: {
        thecloud.close.connect(cloudHide);
        contextMenu.selectedOption.connect(manageContextResponse);
//        showWorkingScreen();
    }

    function cloudHide() {
        console.log("Hide test");
        thecloud.visible = false;
        scene.visible = true;
        menu.visible = true;
    }
    function cloudShow() {
        console.log("Show");
        thecloud.visible = true;
        scene.visible = false;
        menu.visible = false;
    }
    function addPocket(_name,_url,_pocketId,_left,_top,_width,_height,_rotation) {
        renderArea.xmlThumbModel.append(   {name:_name,
                                 imageFileName:_url,
                                 _left:_left,
                                 _top:_top,
                                 _width:_width,
                                 _height:_height,
                                 _rotation:_rotation,
                                 idpocket:_pocketId,
                                 _visible:true,
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
        if (index == -1) return;
        renderArea.xmlThumbModel.remove(index);
    }

    function movePocket(_pocketId,_left,_top) {

        var index = getIndex(_pocketId);

//        console.warn("found index:"+index);
        if (index !== -1) {

//            console.warn("object moved from ("+renderArea.xmlThumbModel.get(index)._left+","+renderArea.xmlThumbModel.get(index)._top+") to ("+_left+","+_top+")");
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
            refreshPocket(_pocketId);
    //        console.log("object sized to ("+_width+","+_height+")");
        }
    }
    function showPocket(_pocketId) {

        var index = getIndex(_pocketId);

    //    console.log("found index:"+index);
        if (index !== -1) {

            renderArea.xmlThumbModel.get(index)._visible = true;
    //        console.log("object sized to ("+_width+","+_height+")");
        }
    }
    function hidePocket(_pocketId) {

        var index = getIndex(_pocketId);

    //    console.log("found index:"+index);
        if (index !== -1) {

            renderArea.xmlThumbModel.get(index)._visible = false;
    //        console.log("object sized to ("+_width+","+_height+")");
        }
    }
    function setRotPocket(_pocketId,angle) {

        var index = getIndex(_pocketId);

    //    console.log("found index:"+index);
        if (index !== -1) {

            renderArea.xmlThumbModel.get(index)._rotation = angle;
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
//            console.log("get:",id,renderArea.xmlThumbModel.get(i).idpocket);
            if (renderArea.xmlThumbModel.get(i).idpocket == id) {
                return i;
            }
        }
        return -1;
    }


}
