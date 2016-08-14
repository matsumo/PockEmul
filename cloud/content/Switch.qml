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

//![0]
import QtQuick 2.0

Item {
    id: toggleswitch
    property bool saveToSettings: true

    property alias objectName: toggleswitch.objectName
    objectName: "checkbox"
    property alias bwidth: background.width
    property alias bheight: background.height
    width: background.width; height: background.height
    signal toggleState(string key,string value)

//![1]
    property bool on: false
//![1]

//![2]
    function toggle() {
        if (toggleswitch.state == "on")
            toggleswitch.state = "off";
        else 
            toggleswitch.state = "on";

        toggleswitch.toggleState(toggleswitch.objectName, toggleswitch.state )
    }
//![2]
    Component.onCompleted: {
        if (saveToSettings) {
            state = cloud.getValueFor(toggleswitch.objectName, defaultText)
        }
    }
//![3]
    function releaseSwitch() {
        if (knob.x == 1) {
            if (toggleswitch.state == "off") return;
        }
        if (knob.x == background.width-knob.width) {
            if (toggleswitch.state == "on") return;
        }
        toggle();
    }
//![3]

//![4]
    Image {
        id: background
        source: "images/background-off.png"
        MouseArea { anchors.fill: parent; onClicked: toggle() }
    }
//![4]

//![5]
    Image {
        id: knob
        x: 1; y: 1
        source: "images/knob.png"
        width: background.height - 2
        height: background.height - 2

        MouseArea {
            anchors.fill: parent
            drag.target: knob; drag.axis: Drag.XAxis; drag.minimumX: 1; drag.maximumX: background.width-knob.width
            onClicked: toggle()
            onReleased: releaseSwitch()
        }
    }
//![5]

//![6]
    states: [
        State {
            name: "on"
            PropertyChanges { target: knob; x: background.width-knob.width }
            PropertyChanges { target: background; source: "images/background-on.png"; }
            PropertyChanges { target: toggleswitch; on: true }
        },
        State {
            name: "off"
            PropertyChanges { target: knob; x: 1 }
            PropertyChanges { target: toggleswitch; on: false }
        }
    ]
//![6]

//![7]
    transitions: Transition {
        NumberAnimation { properties: "x"; easing.type: Easing.InOutQuad; duration: 200 }
    }
//![7]
}
//![0]
