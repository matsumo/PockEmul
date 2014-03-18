import QtQuick 1.0

Item {
    id: delegateItem
    x: 0
    z: PathView.z
    width: height / 2
    height: parent.height
    scale: PathView.iconScale

    // The coverflow item consists of two images with white borders.
    // The second image is flipped and has some opacity for nice mirror effect.
    Column  {
        id: delegate
        y: coverFlow.topMargin
        spacing: 5

        // White borders around the image
        Rectangle {
            id: delegateImage

            // The rectangle is a square.
            width: delegateItem.width
            height: delegateImage.width
            color: dlgImg.status == Image.Ready ? "white" : "transparent"

            // Should go on top of the reflection image when zooming.
            z: reflection.z + 1

            Image {
                id: dlgImg

                width: delegateImage.width - 8
                height: delegateImage.height - 8
                anchors.centerIn: parent
                clip: true

                // Don't stretch the image, and use asynchronous loading.
                fillMode: Image.PreserveAspectCrop
                asynchronous: true

                // The image will be provided by the model
                source: url

                // Smoothing slows down the scrolling even more. Use it with consideration.
                smooth: true
            }
        }

        // Reflection
        Item {
            width: delegateImage.width
            height: delegateImage.height

            Image {
                id: reflection

                width: delegateImage.width
                height: delegateImage.height
                anchors.centerIn: parent
                clip: true

                // The reflection uses the same image as the delegateImage.
                // This way there's no need to ask the image again from the provider.
                source: dlgImg.source
                opacity: 0.6

                fillMode: Image.PreserveAspectCrop
                asynchronous: true
                smooth: true  // Use with consideration.

                transform : Scale {
                    yScale: -1
                    origin.y: delegateImage.height / 2
                }

                // NOTE: This does not work when there's 3D transformations
                // (like rotation around Y-axis or X-axis)
                //AlphaGradient {}
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            if (pathView.currentIndex == index) {
                console.log("Clicked on current item, zoom it")
                parent.state == "scaled" ? parent.state = "" : parent.state = "scaled"
            } else {
                console.log("Clicked on item at index " + index + ", focusing it")
                pathView.currentIndex = index;
            }
        }
    }

    // Rotation depends on the item's position on the PathView.
    // I.e. nicely rotate the image & reflection around Y-axis before disappearing.
    transform: Rotation {
        origin.x: delegateImage.width / 2
        origin.y: delegateImage.height / 2
        axis { x: 0; y: 1; z: 0 } // Rotate around y-axis.
        angle: PathView.angle
    }

    // States and transitions for scaling the image.
    states: [
        State {
            name: "scaled"
            PropertyChanges {
                target: delegateImage
                // Scale up the icon
                scale: 1.8
            }
        }
    ]

    transitions: [
        Transition {
            from: ""
            to: "scaled"
            reversible: true
            ParallelAnimation {
                PropertyAnimation {
                    target: delegateImage
                    properties: "scale"
                    duration: 300
                }
            }
        }
    ]
}
