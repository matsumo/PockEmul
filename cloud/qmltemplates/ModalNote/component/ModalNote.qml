import QtQuick 1.0

// ModalNote Item consists of Fader on one rectangle, two states and animations between them.
// - Fader: Dimms the background & captures mouse events.
// - Rect: Shows a dialog graphics with some text on top of it.
// - States: Has the positions defined to which the note dialog smoothly slides
//   with transitions.
// - Transitions: Slides in and shows the dialog for few seconds, then slides it out.
Item {
    id: modalNote

    // The width and height are meant to be replaced by the parent, e.g. by
    // using anchors.fill. So remember to modify these when instantiating
    // a ModalNote. Otherwise it'll just use S³ default resolution.
    width: 360
    height: 640

    property string text: "NOT SET"
    property int fontPointSize: 8
    property int noteShowTime: 2000 // In milliseconds.

    // This function will start showing the note, i.e. changes the note's state to show,
    // show it for few seconds, and then hide it again.
    function show () {
        modalNote.state = "show"
    }

    // Use Fader component to dim the background & prevent key/touch events
    // to items lying underneath the dialog. I.e. makes the dialog "modal".
    Fader {
        id: fadeRect

        anchors.fill: parent
        // Bind dialog's own states with fader's corresponding states.
        state: modalNote.state == "show" ? "faded" : ""
        // Use slightly faster transition time than for the main dialog
        transitionDuration: 300
    }

    // This rectangle shows some "dialog graphics & text".
    Rectangle {
        id: dialog

        // Keep the dialog hidden before sliding it in
        x: -200
        // This keeps the dialog vertically centered on the screen, so that
        // it slides only on the x-axis.
        anchors.verticalCenter: modalNote.verticalCenter

        Image {
            id: dlgImage
            source: "gfx/button_generic.png"
            anchors.centerIn: dialog
        }

        Text {
            id: dlgText
            text: modalNote.text
            font.pointSize: fontPointSize
            anchors.centerIn: dlgImage

            anchors.leftMargin: 10
            anchors.topMargin: 10
            wrapMode: Text.WordWrap
        }
    }

    // Originally, keep the dialog hidden. When "show" state is being entered,
    // slide in the msg dialog from left to center.
    // Fader has it's own states and transitions, which are triggered parallerly.
    states: [
        State {
            name: ""
            PropertyChanges {
                target: dialog
                // Move the dialog over the right edge of the screen.
                x: modalNote.width+200
            }
        },
        State {
            name: "show"
            PropertyChanges {
                target: dialog
                // Center the dialog on the x-axis.
                x: modalNote.width / 2
            }
        }
    ]

    // The transitions handle showing the dialog for few seconds and then automatically
    // trigger the state change back to the main state (=hidden) with ScriptAction element.
    transitions: [
        Transition {
            from: ""
            to: "show"
            SequentialAnimation {
                // Slide in the text dialog.
                PropertyAnimation { properties: "x"; from:-200; duration: 400; easing.type: Easing.OutBounce;}
                // Pause for a while so that user can read the note and then trigger slide out.
                PauseAnimation { duration: noteShowTime }
                ScriptAction { script: modalNote.state=""; }
            }
        },
        Transition {
            from: "show"
            to: ""
            PropertyAnimation { properties: "x"; to: modalNote.width+200; duration: 400; easing.type: Easing.OutCubic;}
        }
    ]
}
