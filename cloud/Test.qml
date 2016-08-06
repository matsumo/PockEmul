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
//import QtQuick.XmlListModel 2.0
import QtQuick.Window 2.1
import QtQuick.Controls 1.2
import "content"
import "TabbedQuickApp"
import "."


Rectangle {
    id: renderArea

    signal sendWarning(string test)
    signal sendKeyPressed(string id,int key, int mod,int scancode)
    signal sendKeyReleased(string id,int key, int mod,int scancode)
    signal sendContextMenu(string id,int x,int y)
    signal sendDisableKeyboard(string id)
    signal sendEnableKeyboard(string id)
    signal sendClick(string pocketid,int touchid,int x,int y)
    signal sendUnClick(string pocketid,int touchid,int x,int y)
    signal sendDblClick(string id,int x,int y)
    signal sendMovePocket(string id,int x,int y)
    signal sendMoveAllPocket(int x,int y)
    signal setZoom(int x,int y,real z)
    signal sendRotPocket(string id,int rotation)
    signal maximize(string id)
    signal minimize(string id)
    signal fit();

    signal sendNewPocket();
    signal sendLoadPocket(string id);
    signal sendNewExt();
    signal sendDev();
    signal sendSave();
    signal sendLoad();
    signal sendBook();
    signal sendCloseAll();
    signal sendCheck();
    signal sendExit();

    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160); }

     property alias xmlThumbModel: xmlThumbModel

    visible: true
    width: 1024; height: 600
//    anchors.fill: parent
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
                        repeater.itemAt(i).touchEnabled = true;
                        sendEnableKeyboard(renderArea.xmlThumbModel.get(i).idpocket);
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
//                    console.log("send key:",event.key);
                    sendKeyPressed(idpocket,event.key,event.modifiers,event.nativeScanCode);
                    event.accepted = true;
                }
                Keys.onReleased: {
                    sendKeyReleased(idpocket,event.key,event.modifiers,event.nativeScanCode);
                    event.accepted = true;
                }
                Keys.onContext1Pressed: console.log("okkkk");

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
                    property real previousScale: 1
                    enabled: false //!parent.touchEnabled
                    anchors.fill: parent
                    pinch.target: photoFrame
                    pinch.minimumRotation: -360
                    pinch.maximumRotation: 360
                    pinch.minimumScale: 0.1
                    pinch.maximumScale: 10
//                    onPinchStarted: previousScale=1;
//                    onPinchUpdated: {
//                        console.warn("pinch local");
//                        setZoom(pinch.startCenter.x,pinch.startCenter.y,pinch.scale);
//                        previousScale=pinch.scale;
//                    }
//                    onRotationChanged:  photoFrame.rotation = pinch.rotation
                }

                MultiPointTouchArea {
                    property bool isdrag: false;
                    property point previousPosition: Qt.point(1, 1);
                    property point previousScenePosition: Qt.point(1, 1);

                    id: pocketTouchArea
                    enabled: parent.touchEnabled
                    mouseEnabled: true;
                    anchors.fill: parent
                    Timer{
                                id:timer
                                interval: 200
//                                onTriggered: singleClick()
                            }
                    onPressed: {
                        photoFrame.focus = true;
                        if ((touchPoints.length===1) && !main.keyAt(idpocket,touchPoints[0].x,touchPoints[0].y)) {
                            isdrag=true;
                            previousPosition = Qt.point(touchPoints[0].x, touchPoints[0].y);
                            previousScenePosition = Qt.point(touchPoints[0].sceneX, touchPoints[0].sceneY);
                            console.log("previous:",previousPosition.x,previousPosition.y);
                            if(timer.running) {
//                                console.log("context")
                                isdrag=false;
                                if (false) {
                                    sendContextMenu(idpocket,touchPoints[0].x,touchPoints[0].y);
                                }
                                else {
                                    contextMenu.idpocket = idpocket;
                                    contextMenu.width = 300*cloud.getValueFor("hiResRatio","1");//Math.max(300,image.width/2);
                                    contextMenu.mousePt = Qt.point(touchPoints[0].x,touchPoints[0].y);
//                                    contextMenu.buttons = mouse.buttons;
                                    contextMenu.popup(photoFrame.x+touchPoints[0].x, photoFrame.y+touchPoints[0].y);
                                    console.log("popup:",contextMenu.selectedOption);
                                }

                                timer.stop()
                            }
                            else
                                timer.restart()
                        }

                        if (touchPoints.length !== 1) {
                            isdrag = false;
                        }

                        for (var i=0;i<touchPoints.length;i++) {
                            if (touchPoints[i].pointId >= 0) {
                                var tx = touchPoints[i].x;
                                var ty = touchPoints[i].y;
//                                console.warn("Multitouch pressed touch", touchPoints[i].pointId, "at", tx, ",", ty)
                                sendClick(idpocket,touchPoints[i].pointId,touchPoints[i].x,touchPoints[i].y);
                            }
                        }
                    }
                    onReleased: {
                        if ( isdrag && (touchPoints.length === 1)) {
                            console.warn("SWIPE Release");
                            if(timer.running) {
                                var deltax = -(touchPoints[0].sceneX - previousScenePosition.x);
                                var deltay = -(touchPoints[0].sceneY - previousScenePosition.y);
                                console.log("YES:",idpocket,touchPoints[0].sceneX,touchPoints[0].sceneY,deltax,deltay);

                                if (Math.abs(deltax) > 40 || Math.abs(deltay) > 40) {
                                    if (deltax > 30 && Math.abs(deltay) < 30) {
                                        // swipe right
                                        console.log("Swipe Right");
                                        main.flip(idpocket,2);
                                    } else if (deltax < -30 && Math.abs(deltay) < 30) {
                                        // swipe left
                                        console.log("swipeLeft");
                                        main.flip(idpocket,1);
                                    } else if (Math.abs(deltax) < 30 && deltay > 30) {
                                        // swipe down
                                        console.log("Swipe Down")
                                        main.flip(idpocket,3);
                                    } else if (Math.abs(deltax) < 30 && deltay < 30) {
                                        // swipe up
                                        console.log("swipeUp");
                                        main.flip(idpocket,0);
                                    }
                                }
                                //                                timer.stop()
                            }
//                            else
//                                timer.restart()

                        }
                        isdrag = false;

                        //                    console.warn("touchPoints count:",touchPoints.length);
                        for (var i=0;i< touchPoints.length;i++) {
                            var tx = touchPoints[i].x;
                            var ty = touchPoints[i].y;
//                            console.warn("Multitouch released touch", touchPoints[i].pointId, "at", tx, ",", ty)
                            sendUnClick(idpocket,touchPoints[i].pointId,tx,ty);
                        }

                   }
                    onCanceled: {
                        isdrag = false;
                        for (var i=0;i< touchPoints.length;i++) {
                            var tx = touchPoints[i].x;
                            var ty = touchPoints[i].y;
//                            console.warn("Multitouch canceled touch", touchPoints[i].pointId, "at", tx, ",", ty)
                            sendUnClick(idpocket,touchPoints[i].pointId,tx,ty);
                        }
                    }
                    onTouchUpdated: {
                        if ( (touchPoints.length === 0)) {
//                            console.warn("SWIPE updated");
                        }

                        if (touchPoints.length !== 1) {
                            isdrag = false;
                        }
                        // Check for move if only one touchpoint
                        if (isdrag && (touchPoints.length===1)) {
                            var x = touchPoints[0].x;
                            var y = touchPoints[0].y;
                            var dx = x - previousPosition.x;
                            var dy = y - previousPosition.y;
//                            previousPosition = Qt.point(touchPoints[0].x, touchPoints[0].y);
//                            console.warn("Multitouch updated:", touchPoints[0].pointId, "at", x,",",y,"d=",dx, ",", dy)
                            sendMovePocket(idpocket,photoFrame.x+dx,photoFrame.y+dy);
                        }
                    }

//                    onGestureStarted:  gesture.cancel();



                MouseArea {
                    property bool isdrag: false;
                    property point previousPosition: Qt.point(1, 1);

                    id: dragArea
                    enabled: false;
                    hoverEnabled: true;
                    preventStealing: false
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    propagateComposedEvents: true

                    //                        onPressed: {
                    //                            //                        console.warn("clik");
                    //                            previousPosition = Qt.point(mouse.x, mouse.y);
                    //                            photoFrame.focus = true;
                    //                            if (mouse.button === Qt.RightButton) {
                    //                                if (!main.keyAt(idpocket,mouse.x,mouse.y)) {
                    //                                    isdrag=false;
                    //                                    sendContextMenu(idpocket,mouse.x,mouse.y);
                    //                                }
                    //                                else {
                    //                                    isdrag=true;
                    //                                }
                    //                            }
                    //                            if (mouse.button === Qt.LeftButton) {
                    //                                if (!main.keyAt(idpocket,mouse.x,mouse.y)) {
                    //                                    isdrag=true;
                    //                                }

                    //                                sendClick(idpocket,0,mouse.x,mouse.y);
                    //                            }
                    //                            mouse.accepted=true;
                    //                        }

                    //                        onReleased: {
                    //                            isdrag=false;
                    ////                            sendUnClick(idpocket,mouseX,mouseY);
                    //                            sendUnClick(idpocket,0,previousPosition.x,previousPosition.y);
                    //                            mouse.accepted=true;

                    //                        }

                    onDoubleClicked: {
                        isdrag = false;
                        if (main.keyAt(idpocket,mouse.x,mouse.y)) {
                            sendDblClick(idpocket,mouseX,mouseY);
                        }
                        else {
                            contextMenu.idpocket = idpocket;
                            contextMenu.width = 300*cloud.getValueFor("hiResRatio","1");//Math.max(300,image.width/2);
                            contextMenu.mousePt = Qt.point(mouse.x,mouse.y);
                            contextMenu.buttons = mouse.buttons;
                            contextMenu.popup(photoFrame.x+mouseX, photoFrame.y+mouseY);
                            console.log("popup:",contextMenu.selectedOption);
                        }
                    }

                    //                        onPositionChanged: {
                    //                            if (isdrag) {
                    //                                var dx = mouse.x - previousPosition.x
                    //                                var dy = mouse.y - previousPosition.y
                    //                                sendMovePocket(idpocket,photoFrame.x+dx,photoFrame.y+dy);
                    //                            }
                    //                            else {
                    //                                if (main.keyAt(idpocket,mouse.x,mouse.y)) {
                    //                                    cursorShape = Qt.PointingHandCursor;
                    //                                }
                    //                                else {
                    //                                    cursorShape = Qt.ArrowCursor;
                    //                                }
                    //                            }
                    //                            mouse.accepted=true;

                    //                        }
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

        Rectangle {
            id: menuSquare
            anchors.top: parent.top
//            anchors.bottom: parent.bottom
            anchors.left: parent.left
            z: 9999 //parent.z

            width: dp(48)
            height: dp(48)
            color: "black"

            Rectangle {
                anchors.top: parent.top
                anchors.topMargin: dp(16)
                anchors.left: parent.left
                anchors.leftMargin: dp(14)
                width: dp(20)
                height: dp(2)
            }

            Rectangle {
                anchors.top: parent.top
                anchors.topMargin: dp(23)
                anchors.left: parent.left
                anchors.leftMargin: dp(14)
                width: dp(20)
                height: dp(2)
            }

            Rectangle {
                anchors.top: parent.top
                anchors.topMargin: dp(30)
                anchors.left: parent.left
                anchors.leftMargin: dp(14)
                width: dp(20)
                height: dp(2)
            }

            MouseArea {
                anchors.fill: parent

                onClicked: {
                    console.log("menuSquare: click",new Date());
                    nav.toggle()
                }
            }
        }

        NavigationDrawer {
            id: nav
            enabled: true
            parent: renderArea
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            color: "red"
            border.color: "white"
            border.width: 5

            position: Qt.LeftEdge

            Launchmenu {
                id:menu2
            }
        }
    }

    ShowRoom {
        id: showroomNew
        z: 9999
        visible: false
        anchors.fill: parent
        source: "qrc:/pockemul/config.xml"
    }

    ShowRoom {
        id: showroomExt
        z: 9999
        visible: false
        anchors.fill: parent
        source: "qrc:/pockemul/configExt.xml"
    }

    Main {
        id: thecloud
        anchors.fill: parent
        visible: false;
    }

    VisualItemModel {
        id: settingsModel
        Tab {
            name: "Settings"
            icon: "pics/back-white.png"

            color: "red"

            Settings {
                anchors.fill: parent
            }
        }
    }

    TabbedUI {
        id: settings
        visible: false
        z: 9999
        tabsHeight: 72 * cloud.getValueFor("hiResRatio","1")
        tabIndex: 0
        tabsModel: settingsModel
        quitIndex: 0
        onClose: visible=false;
    }

    VisualItemModel {
        id: aboutModel
        Tab {
            name: "Help"
            icon: "pics/help.png"
            About {
                id: helpFlick
                anchors.fill: parent
                fileName: ":/pockemul/help.html"
            }
        }
        Tab {
            name: "About PockEmul "+Qt.application.version
            icon: "pics/white-about-256.png"
            About {
                id: aboutFlick
                anchors.fill: parent
                fileName: ":/pockemul/release_notes.html"
            }
        }
        Tab {name: "Back"
            icon: "pics/back-white.png"
            MouseArea {
                anchors.fill: parent
                onClicked: close();
            }
        }
    }

    TabbedUI {
        id: about
        visible: false
        z: 9999
        tabsHeight: 72 * cloud.getValueFor("hiResRatio","1")
        tabIndex: 0
        tabsModel: aboutModel
        quitIndex: 2
        onClose: visible=false;

    }


    ContextMenu {
        id: contextMenu
    }

    Download {
        id: download
        visible : false
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

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
                    width: Math.min(workingScreen.width,workingScreen.height) / 4
                    height: Math.min(workingScreen.width,workingScreen.height) / 4
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
            sendDisableKeyboard(idpocket);
            repeater.itemAt(ind).touchEnabled = !repeater.itemAt(ind).touchEnabled;
        }

    }

    Component.onCompleted: {
        thecloud.close.connect(cloudHide);
        contextMenu.selectedOption.connect(manageContextResponse);
//        showWorkingScreen();

        console.log("Test.qml: Completed",new Date());
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


    function setDownloadProgress(min,max,val) {
        download.minimumValue = min;
        download.maximumValue = max;
        download.value = val;
    }

    function setDownloadVisible(visible) {
        download.visible = visible;
    }

    function pickExtension(brand) {
        showroomExt.brandsearch = brand;
        showroomExt.visible = true;
    }
    function pickExtensionConnector(connector) {
        showroomExt.connectorsearch = connector;
        showroomExt.pobjectsmodel.reload();
        showroomExt.visible = true;
    }

}
