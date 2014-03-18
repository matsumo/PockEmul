import QtQuick 1.0
import "../component" as Comp

Item {
    width: 640
    height: 360

    // Pre-defined list of images to be shown. The model could be instead
    // e.g. the document gallery model from Qt Mobility in real life app.
    ListModel {
        id: galleryModel

        ListElement { url: "gfx/accordionlist.png" }
        ListElement { url: "gfx/background.png" }
        ListElement { url: "gfx/image_placeholder.png" }
        ListElement { url: "gfx/photo_back.png" }
        ListElement { url: "gfx/rotation_icon.png" }
        ListElement { url: "gfx/splash.png" }
        ListElement { url: "gfx/splash_screen.png" }
        ListElement { url: "gfx/media_browser_visuals.png" }
    }

    // The CoverFlow element itself, which shows the images (and their
    // reflections) on a PathView from the model.
    Comp.CoverFlow {
        anchors {
            fill: parent
        }

        // Provide the model (which provides the pahts to the images)
        // to the CoverFlow component.
        model: galleryModel
        onCurrentIndexChanged: console.log("Current index: " + index)
    }

    // "Exit Button"
    Item {
        width: 64
        height: 64
        anchors {
            top: parent.top
            topMargin: 10
            right: parent.right
            rightMargin: 10
        }

        Text {
            anchors.fill: parent
            color: "black"
            font.pixelSize: 64
            text: "X"
        }
        MouseArea {
            anchors.fill: parent;
            onClicked: Qt.quit();
        }
    }
}
