import QtQuick 1.0

// Accordion list
Item {
    id: container

    // Default width
    width: 360
    // Default height
    height: 640
    // Subitem expansion duration
    property int animationDuration: 100
    // Subitem indentation
    property int indent: 20
    // Scrollbar width
    property int scrollBarWidth: 8
    // Background for list item
    property string bgImage: './gfx/list_item.png'
    // Background image for pressed list item
    property string bgImagePressed: './gfx/list_item_pressed.png'
    // Background image for active list item (currently not used)
    property string bgImageActive: './gfx/list_item_active.png'
    // Background image for subitem
    property string bgImageSubItem: "./gfx/list_subitem.png"
    // Arrow indicator for item expansion
    property string arrow: './gfx/arrow.png'
    // Font properties for top level items
    property string headerItemFontName: "Helvetica"
    property int headerItemFontSize: 12
    property color headerItemFontColor: "black"
    // Font properties for  subitems
    property string subItemFontName: "Helvetica"
    property int subItemFontSize: headerItemFontSize-1
    property color subItemFontColor: "black"

    signal itemClicked(string itemTitle, string subItemTitle)

    AccordionListModel {
        id: mainModel
    }

    ListView {
        id: listView
        height: parent.height
        anchors {
            left: parent.left
            right: parent.right
        }
        model: mainModel
        delegate: listViewDelegate
        focus: true
        spacing: 0
    }

    Component {
        id: listViewDelegate
        Item {
            id: delegate
            // Modify appearance from these properties
            property int itemHeight: 64
            property alias expandedItemCount: subItemRepeater.count

            // Flag to indicate if this delegate is expanded
            property bool expanded: false

            x: 0; y: 0;
            width: container.width
            height: headerItemRect.height + subItemsRect.height

            // Top level list item.
            ListItem {
                id: headerItemRect
                x: 0; y: 0
                width: parent.width
                height: parent.itemHeight
                text: itemTitle
                onClicked: expanded = !expanded

                bgImage: container.bgImage
                bgImagePressed: container.bgImagePressed
                bgImageActive: container.bgImageActive
                fontName: container.headerItemFontName
                fontSize: container.headerItemFontSize
                fontColor: container.headerItemFontColor
                fontBold: true

                // Arrow image indicating the state of expansion.
                Image {
                    id: arrow
                    fillMode: "PreserveAspectFit"
                    height: parent.height*0.3
                    source: container.arrow
                    rotation: expanded ? 90 : 0
                    smooth: true
                    anchors {
                        right: parent.right
                        verticalCenter: parent.verticalCenter
                        rightMargin: 10
                    }
                }
            }

            // Subitems are in a column whose height depends
            // on the expanded status. When not expandend, it is zero.
            Item {
                id: subItemsRect
                property int itemHeight: delegate.itemHeight

                y: headerItemRect.height
                width: parent.width
                height: expanded ? expandedItemCount * itemHeight : 0
                clip: true

                opacity: 1
                Behavior on height {
                    // Animate subitem expansion. After the final height is reached,
                    // ensure that it is visible to the user.
                    SequentialAnimation {
                        NumberAnimation { duration: container.animationDuration; easing.type: Easing.InOutQuad }
                        ScriptAction { script: ListView.view.positionViewAtIndex(index, ListView.Contain) }
                    }
                }

                Column {
                    width: parent.width

                    // Repeater creates each sub-ListItem using attributes
                    // from the model.
                    Repeater {
                        id: subItemRepeater
                        model: attributes
                        width: subItemsRect.width

                        ListItem {
                            id: subListItem
                            width: delegate.width
                            height: subItemsRect.itemHeight
                            text: subItemTitle
                            bgImage: container.bgImageSubItem
                            fontName: container.subItemFontName
                            fontSize: container.subItemFontSize
                            fontColor: container.subItemFontColor
                            textIndent: container.indent
                            onClicked: {
                                console.log("Clicked: "+itemTitle + "/" + subItemTitle)
                                itemClicked(itemTitle, subItemTitle)
                            }
                        }
                    }
                }
            }
        }
    }
}
