import QtQuick 1.0

Item {
    id: coverFlow

    width: 640
    height: 360

    // The model that which the PathView will use as a base when providing
    // the cover flow images. If a different kind of model is desired, it
    // should be simply a matter of changing the model's type.
    property ListModel model
    // Defines, how many items will be shown on the pathview concurrently.
    // Works also with 9 items, just slows down a bit.
    property int itemCount: 7
    // Defines, where the CoverFlowDelegate will be positioned on the Y-axis
    property int topMargin: 80

    // Signalled when the current index changes on the PathView.
    signal currentIndexChanged(int index)

    // Background color. One could also use image instead.
    Rectangle {
        anchors.fill: parent
        color: "#d9d3b5"
    }

    Component.onCompleted: {
        console.log("Model has " + model.count + " images");
        pathView.currentIndex = 0;
    }

    PathView {
        id: pathView

        model: coverFlow.model
        delegate: CoverFlowDelegate {}
        path: coverFlowPath
        pathItemCount: coverFlow.itemCount

        anchors.fill: parent
        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5
        focus: true

        onMovementStarted: {
            console.log("PathView movement started")
        }

        onMovementEnded: {
            console.log("PathView movement ended")
        }

        Keys.onRightPressed: {
            if (interactive) {
                incrementCurrentIndex()
            }
        }

        Keys.onLeftPressed: {
            if (interactive) {
                decrementCurrentIndex()
            }
        }

        onCurrentIndexChanged: {
            coverFlow.currentIndexChanged(pathView.currentIndex);
        }
    }

    // The path defining where the items appear and how they move around.
    //
    // Note: The x-axis value is defined in percentages of the
    // screen width like below. The path is a straight horizontal line.
    Path {
        id: coverFlowPath

        // "Start zone"
        startX: -25
        startY: coverFlow.height / 2
        PathAttribute { name: "z"; value: 0 }
        PathAttribute { name: "angle"; value: 70 }
        PathAttribute { name: "iconScale"; value: 0.6 }

        // Just before middle
        PathLine { x: coverFlow.width * 0.35; y: coverFlow.height / 2;  }
        PathAttribute { name: "z"; value: 50 }
        PathAttribute { name: "angle"; value: 45 }
        PathAttribute { name: "iconScale"; value: 0.85 }
        PathPercent { value: 0.40 }

        // Middle
        PathLine { x: coverFlow.width * 0.5; y: coverFlow.height / 2;  }
        PathAttribute { name: "z"; value: 100 }
        PathAttribute { name: "angle"; value: 0 }
        PathAttribute { name: "iconScale"; value: 1.0 }

        // Just after middle
        PathLine { x: coverFlow.width * 0.65; y: coverFlow.height / 2; }
        PathAttribute { name: "z"; value: 50 }
        PathAttribute { name: "angle"; value: -45 }
        PathAttribute { name: "iconScale"; value: 0.85 }
        PathPercent { value: 0.60 }

        // Final stop
        PathLine { x: coverFlow.width + 25; y: coverFlow.height / 2; }
        PathAttribute { name: "z"; value: 0 }
        PathAttribute { name: "angle"; value: -70 }
        PathAttribute { name: "iconScale"; value: 0.6 }
        PathPercent { value: 1.0 }
    }
}
