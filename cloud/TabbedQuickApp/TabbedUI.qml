import QtQuick 2.0

Rectangle {
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

        anchors.top: parent.top
	anchors.bottom: tabBar.top

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
	    height: tabs.height
	    width: tabs.width / tabsModel.count

	    color: "transparent"

	    Image {
		source: tabsModel.children[index].icon
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		anchors.topMargin: 4
	    }

	    Text {
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 4
		color: "white"
		text: tabsModel.children[index].name
	    }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (index==quitIndex) {
                    tabClicked(1);
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
	height: tabsHeight
	width: parent.width

	// take the whole parent width
	anchors.left: parent.left
	anchors.right: parent.right

	// attach it to the view bottom
	anchors.bottom: parent.bottom

	id: tabBar

	gradient: Gradient {
	    GradientStop {position: 0.0; color: "#666666"}
	    GradientStop {position: 1.0; color: "#000000"}
	}

	// place all the tabs in a row
	Row {
	    anchors.fill: parent

            id: tabs

	    Repeater {
		model: tabsModel.count

		delegate: tabBarItem
	    }
        }
    }
}
