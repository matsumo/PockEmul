import QtQuick 2.0
import QtGraphicalEffects 1.0

Rectangle {
    property bool horizontal: true
    property bool vertical: false

    onHorizontalChanged: vertical = !horizontal
    onVerticalChanged: horizontal = !vertical

    // width of the tab bar
    property int tabsWidth : 128

    // height of the tab bar
    property int tabsHeight : 64

    // index of the active tab
    property int tabIndex : 0

    // the model used to build the tabs
    property VisualItemModel tabsModel

    property int quitIndex : 999999
    signal close;

    anchors.fill: parent

    // will contain the tab views
    Rectangle {
        id: tabViewContainer
        width: parent.width
        height: parent.height
        color: "red"

        anchors.top: parent.top
        anchors.bottom: horizontal ? tabBar.top : parent.bottom
        anchors.left: vertical ? tabBar.right : parent.left
        anchors.right: parent.right

        // build all the tab views
        Repeater {
            model: tabsModel
        }
    }

    Component.onCompleted:
    {
	// hide all the tab views
	for(var i = 0; i < tabsModel.children.length; i++)
	{
	    tabsModel.children[i].visible = false;
	}
	// select the default tab index
	tabClicked(tabIndex);
    }

    function tabClicked(index)
    {
	// unselect the currently selected tab
	tabs.children[tabIndex].color = "transparent";

	// hide the currently selected tab view
	tabsModel.children[tabIndex].visible = false;

	// change the current tab index
	tabIndex = index;

	// highlight the new tab
	tabs.children[tabIndex].color = "#30ffffff";

	// show the new tab view
	tabsModel.children[tabIndex].visible = true;
    }

    Component {
	id: tabBarItem

	Rectangle {
        height: horizontal ? tabsHeight : (tabs.height / tabsModel.count)
        width: horizontal ? (tabs.width / tabsModel.count) : tabsWidth
//	    height: tabs.height
//	    width: tabs.width / tabsModel.count
        property int margin : tabs.height / 16

	    color: "transparent"

	    Image {
		source: tabsModel.children[index].icon
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
        anchors.topMargin: margin
        height: (parent.height - 2*margin)/2
        width: (parent.height - 2*margin)/2
        }

	    Text {
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
        anchors.bottomMargin: margin
        height: (parent.height - 2*margin)/2
		color: "white"
		text: tabsModel.children[index].name
        renderType: Text.NativeRendering
	    }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (index==quitIndex) {
//                    tabClicked(1);
                    close();
                }
                else
                    tabClicked(index);
            }
        }
	}
    }

    // the tab bar
    Rectangle {
        height: horizontal ? tabsHeight : parent.height
        width: horizontal ? parent.width : tabsWidth
        color: "black"
//        border.color: "red"

        // take the whole parent width
        anchors.left: parent.left
//        anchors.right: parent.right

        // attach it to the view bottom
        anchors.bottom: parent.bottom

        id: tabBar

        LinearGradient {
            visible: horizontal
            anchors.fill: parent
            start: horizontal ? Qt.point(0,0) : Qt.point(parent.width,0)
            end: horizontal ? Qt.point(0,parent.height) : Qt.point(0,0)
            gradient: Gradient {
                GradientStop { position: 0.0; color: "#666666" }
                GradientStop { position: 1.0; color: "#000000" }
            }
        }
//        gradient: Gradient {
//            GradientStop {position: 0.0; color: "#666666"}
//            GradientStop {position: 1.0; color: "#000000"}
//        }

        // place all the tabs in a row
        Grid {
            columns: horizontal ? tabsModel.count : 1
            anchors.fill: parent

                id: tabs

            Repeater {
            model: tabsModel.count

            delegate: tabBarItem
            }
         }

    }
}
