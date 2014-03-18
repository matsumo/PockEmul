import QtQuick 1.0

Rectangle {
    width: 360
    height: 640

    id: root

    VisualItemModel {
	id: tabsModel
	Tab {
        name: "Own Cloud"
	    icon: "pics/tab0.png"

	    color: "yellow"

            Text {
		anchors.centerIn: parent
                text: "This is page 1"
            }
        }
	Tab {
        name: "Public Cloud"
	    icon: "pics/tab1.png"

	    color: "green"
            Text {
		anchors.centerIn: parent
                text: "This is page 2"
            }
        }
	Tab {
        name: "Settings"
	    icon: "pics/tab2.png"

	    color: "red"
            Text {
		anchors.centerIn: parent
                text: "This is page 3"
            }
        }
    }

    TabbedUI {
	tabsHeight: 72
	tabIndex: 1
        tabsModel: tabsModel
    }

    Image {
	source: "pics/close.png"
	anchors.right: parent.right
	anchors.top: parent.top
	anchors.margins: 10

	MouseArea {
	    anchors.fill: parent
	    onClicked: Qt.quit()
	}
    }
}
