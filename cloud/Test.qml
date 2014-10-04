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
import QtQuick 2.0
import QtQuick.XmlListModel 2.0
import QtQuick.Window 2.1



Window {
    id: testarea

     property alias xmlThumbModel: xmlThumbModel

    visible: true
    width: 1024; height: 600
    color: "black"

    property int highestZ: 0
    property real defaultSize: 200

    ListModel {
        id: xmlThumbModel
    }


    Repeater {
        model: xmlThumbModel

        Rectangle {
            id: photoFrame
            width: _width * image.scale //+ 20
            height: _height * image.scale //+ 20
//            border.color: "black"
            color: "transparent"

            border.width: 0
            smooth: true
            antialiasing: true
            x: _left //Math.random() * testarea.width - defaultSize
            y: _top //Math.random() * testarea.height - defaultSize
            rotation: Math.random() * 13 - 6
            Image {
                id: image
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                source: "image://Pocket/"+idpocket+"/"+dummy
                scale: 1 //defaultSize / Math.max(sourceSize.width, sourceSize.height)
                antialiasing: true
            }
            PinchArea {
                anchors.fill: parent
                pinch.target: photoFrame
                pinch.minimumRotation: -360
                pinch.maximumRotation: 360
                pinch.minimumScale: 0.1
                pinch.maximumScale: 10
                onPinchFinished: photoFrame.border.color = "black";
                MouseArea {
                    id: dragArea
                    hoverEnabled: true
                    anchors.fill: parent
                    drag.target: photoFrame
                    onPressed: {
                        photoFrame.z = ++testarea.highestZ;
                        root.sendClick(idpocket,mouseX,mouseY);
                    }
                    onReleased: root.sendUnClick(idpocket,mouseX,mouseY)
                    onEntered: photoFrame.border.color = "red";
                    onExited: photoFrame.border.color = "black";
                    onWheel: {
                        if (wheel.modifiers & Qt.ControlModifier) {
                            photoFrame.rotation += wheel.angleDelta.y / 120 * 5;
                            if (Math.abs(photoFrame.rotation) < 4)
                                photoFrame.rotation = 0;
                        } else {
                            photoFrame.rotation += wheel.angleDelta.x / 120;
                            if (Math.abs(photoFrame.rotation) < 0.6)
                                photoFrame.rotation = 0;
                            var scaleBefore = image.scale;
                            image.scale += image.scale * wheel.angleDelta.y / 120 / 10;
                            _left -= image.width * (image.scale - scaleBefore) / 2.0;
                            _top -= image.height * (image.scale - scaleBefore) / 2.0;
                        }
                    }
                }
            }
        }
    }

//    Image {
//        anchors.top: parent.top
//        anchors.left: parent.left
//        anchors.margins: 10
//        source: "core/folder.png"
//        MouseArea {
//            anchors.fill: parent
//            anchors.margins: -10
//            onClicked: fileDialog.open()
//        }
//    }

//    Text {
//        anchors.bottom: parent.bottom
//        anchors.left: parent.left
//        anchors.right: parent.right
//        anchors.margins: 10
//        color: "darkgrey"
//        wrapMode: Text.WordWrap
//        font.pointSize: 8
//        text: "On a touchscreen: use two fingers to zoom and rotate, one finger to drag\n" +
//              "With a mouse: drag normally, use the vertical wheel to zoom, horizontal wheel to rotate, or hold Ctrl while using the vertical wheel to rotate"
//    }

//    Component.onCompleted: xmlThumbModel.source= "qrc:/pockemul/config.xml"
}
