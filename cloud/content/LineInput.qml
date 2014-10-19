import QtQuick 2.0
import ".."

Rectangle {

    property alias objectName: input.objectName
    property alias text: input.text
    property alias label: label.text
    property alias maximumLength: input.maximumLength
    property alias echoMode: input.echoMode

    property string defaultText

    property alias font: fontstyle.font

    id: lineinput
    signal inputAccepted(string key,string value)



    width: 480;
    height: row.height +5

    Text {
        id: fontstyle
        text: ""
        visible: false
        font.pointSize: 16
    }
    Row {
        id: row
        width: parent.width
//        y: 10
        spacing: 10
        Text {
            id: label
            text: "Label"
            font: fontstyle.font
            anchors.verticalCenter: parent.verticalCenter
        }

        FocusScope {
            width: parent.width - 30 - label.width
            height: label.height ;// * 1.5
            anchors.verticalCenter: parent.verticalCenter

//            BorderImage {
//                source: "images/lineedit-bg.png"
//                border { left: 3; top: 3; right: 3; bottom: 3 }
//                     horizontalTileMode: BorderImage.Stretch
//                     verticalTileMode: BorderImage.Stretch
//                anchors.fill: parent
//            }
            Rectangle {
                id: borderTextInput
                anchors.fill: parent
                border.color: rootCloud.inputBorderColor
                radius: 5
            }
            Rectangle {
                width: borderTextInput.width
                height: borderTextInput.height*0.75
                border.color: rootCloud.backGroundColor
                radius: 5

            }

            TextInput {

                width: parent.width - 20
//                height: parent.height - 10
                anchors.centerIn: parent
                id: input
                objectName: "inputField"
                text: cloud.getValueFor(input.objectName, defaultText)
                font: fontstyle.font

                onTextChanged: {
                    lineinput.inputAccepted(input.objectName, input.text)
                    //Here is where we save the value or string
//                    cloud.saveValueFor(input.objectName, input.text)

                }
            }
        }
    }
}
