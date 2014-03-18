import QtQuick 1.0

Rectangle {
    // Added +2 to dimensions in order to hinder the tearing effect.
    width: parent.width + 2
    height: parent.height + 2
    anchors.centerIn: parent

    gradient: Gradient {
        GradientStop { position: 0.0; color: Qt.rgba(0,0,0,1.0) }
        GradientStop { position: 0.7; color: Qt.rgba(0,0,0,0.8) }
        GradientStop { position: 1.0; color: Qt.rgba(0,0,0,0.2) }
    }
}
