import QtQuick 1.0

Item {
    id: container

    // Default width
    width: 360
    // Default height
    height: 100
    // Orientation of the TabBar. If true, the bar is horizontal, if false, the bar is vertical.
    property bool wide: true
    // Number of tab buttons in the bar
    property int buttonCount: 4
    // Selected button name, initially empty
    property string selectedButton: ""
    // Font properties for the tab button text
    property string fontName: "Helvetica"
    property int fontSize: 14
    property color fontColor: "black"
    property color backgroundBarColor: "white"
    // 1st tab button graphics and text
    property string button1Background: "gfx/button.png"
    property string button1BackgroundSelected: "gfx/button_pressed.png"
    property string button1Text: "1"
    // 2nd tab button graphics and text
    property string button2Background: "gfx/button.png"
    property string button2BackgroundSelected: "gfx/button_pressed.png"
    property string button2Text: "2"
    // 3rd tab button graphics and text
    property string button3Background: "gfx/button.png"
    property string button3BackgroundSelected: "gfx/button_pressed.png"
    property string button3Text: "3"
    // 4th tab button graphics and text
    property string button4Background: "gfx/button.png"
    property string button4BackgroundSelected: "gfx/button_pressed.png"
    property string button4Text: "4"
    // Tab button margins
    property int margins: 8
    // Calculated gap between buttons
    property int gap: !wide ? (container.height-buttonCount*buttonHeight)/(buttonCount+1) : (container.width-buttonCount*buttonWidth)/(buttonCount+1)
    // Calculated tab button dimensions
    property int buttonWidth: !wide ? container.width : container.height
    property int buttonHeight: !wide ? container.width-2*margins : container.height-2*margins

    signal tabButtonClicked(string targetView, string buttonName);

    // Grid can be used as a row or a column depending on the configuration.
    Grid {
        rows: wide ? 1 : buttonCount
        columns: wide ? buttonCount : 1

	// Anchoring is rather tricky when both orientations are supported.
        anchors {
            left: wide ? container.left : undefined
            right: wide ? container.right : undefined
            verticalCenter: wide ? container.verticalCenter : undefined
            horizontalCenter: !wide ? container.horizontalCenter : undefined
            leftMargin: gap
            rightMargin: gap
        }
        spacing: container.gap

        ImageButton {
            id: button1
            text: button1Text
            buttonName: "page1Button"
            target: "Page1.qml"
            width: buttonWidth
            height: buttonHeight
            active: container.selectedButton === buttonName
            bgImage: button1Background
            bgImagePressed: button1BackgroundSelected
            fontName: container.fontName
            fontSize: container.fontSize
            fontColor: container.fontColor
            onClicked: { container.selectedButton = buttonName; container.tabButtonClicked(target, buttonName) }
        }
        ImageButton {
            id: button2
            text: button2Text
            buttonName: "page2Button"
            target: "Page2.qml"
            width: buttonWidth
            height: buttonHeight
            active: container.selectedButton === buttonName
            bgImage: button2Background
            bgImagePressed: button2BackgroundSelected
            fontName: container.fontName
            fontSize: container.fontSize
            fontColor: container.fontColor
            onClicked: { container.selectedButton = buttonName; container.tabButtonClicked(target, buttonName) }
        }
        ImageButton {
            id: button3
            text: button3Text
            buttonName: "page3Button"
            target: "Page3.qml"
            width: buttonWidth
            height: buttonHeight
            active: container.selectedButton === buttonName
            bgImage: button3Background
            bgImagePressed: button3BackgroundSelected
            fontName: container.fontName
            fontSize: container.fontSize
            fontColor: container.fontColor
            onClicked: { container.selectedButton = buttonName; container.tabButtonClicked(target, buttonName) }
        }
        ImageButton {
            id: button4
            text: button4Text
            buttonName: "page4Button"
            target: "Page4.qml"
            width: buttonWidth
            height: buttonHeight
            active: container.selectedButton === buttonName
            bgImage: button4Background
            bgImagePressed: button4BackgroundSelected
            fontName: container.fontName
            fontSize: container.fontSize
            fontColor: container.fontColor
            onClicked: { container.selectedButton = buttonName; container.tabButtonClicked(target, buttonName) }
        }
    }

}
