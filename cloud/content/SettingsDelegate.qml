
import QtQuick 1.1

Item {
    id: delegate

    property string name
    property string labelString
    property string type
    property bool saveInput: true
    property alias buttonElementEnabled: buttonElement.enabled
    property alias inputText: inputElement.text
    property alias defaultText: inputElement.defaultText
    property alias echoMode: inputElement.echoMode

    signal buttonClicked

    width: delegate.ListView.view.width;
    height: (type == "input") ? inputElement.height :
            (type == "action")? buttonElement.height +5 : labelElement.height +5


    LineInput {
        id: inputElement
        objectName: name
        width: parent.width
        label: labelString
        font.pointSize: 16
        onInputAccepted: if (saveInput) cloud.saveValueFor(key,value)
        visible: (type == "input")
    }
    ComboBox {

        visible: (type=="combo")
    }

    Rectangle {
        width: parent.width
        height: labelElement.height *1.2
        color: "black"
        opacity: 1
        visible: (type == "text")
    }

    Row {
        y:2
        spacing: 10
        Text {
            id: labelElement
            text: labelString
            color: (delegate.ListView.isCurrentItem | (type == "text")) ? "white" : "black"
            font { family: "Helvetica"; pointSize: 16; bold: false }
            anchors.verticalCenter: parent.verticalCenter
            visible: ((type == "text") | (type == "action") | (type == "checkbox"))
        }

        TextButton {
            id: buttonElement

            text: labelString
            objectName: name
            font.pointSize: 16
            visible: (type=="action")
            anchors.verticalCenter: parent.verticalCenter
            onClicked: {
                buttonClicked();
            }
        }
        Switch {
            objectName: name
            bheight: buttonElement.height*.9
            bwidth: bheight * 2
            on: (labelString=="checked") ? true:false
            onToggleState: if (saveInput) cloud.saveValueFor(key,value)
            visible: (type == "checkbox")
        }
    }


//    Rectangle {
//        width: delegate.width; height: 1; color: "#cccccc"
//        anchors.bottom: delegate.bottom
//        visible: delegate.ListView.isCurrentItem ? false : true
//    }
//    Rectangle {
//        width: delegate.width; height: 1; color: "white"
//        visible: delegate.ListView.isCurrentItem ? false : true
//    }

}
