import QtQuick 2.0
import QtMultimedia 5.5

Item {
    width: 320
    height: 510
    Camera {
        id: camera
        imageCapture {
            onImageCaptured: {
                // Show the preview in an Image
                photoPreview.source = preview
            }
            onImageSaved: {
                text.text = qsTr("Last Captured Image (%1):").arg(camera.imageCapture.capturedImagePath)
            }
        }
    }
    Column {
        ListView {
//               anchors.fill: parent
               model: QtMultimedia.availableCameras
               delegate: Text {
                   text: modelData.displayName

                   MouseArea {
//                       anchors.fill: parent
                       onClicked: camera.deviceId = modelData.deviceId
                   }
               }
           }

        Text {
            height: 15
            text: qsTr("Preview (Click to capture):")
        }
        VideoOutput {
            source: camera
            focus: visible // To receive focus and capture key events when visible
            width: 320; height: 240
            MouseArea {
                anchors.fill: parent
                onClicked: camera.imageCapture.capture()
            }
        }
        Text {
            id: text
            height: 15
            text: qsTr("Last Captured Image (none)")
        }
        Image {
            id: photoPreview
            width: 320; height: 240
        }
    }
}
