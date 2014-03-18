import QtQuick 1.0

QtObject {
    id: container
    property Item currentView
    property Item previousView

    property Item root

    property int duration: 250
    property bool running: switchAnimation.running
    property bool direction

    function switchView(newView, leftToRight, instant) {
        if (newView != currentView && !switchAnimation.running) {
            // if the new view has a loadView() function, call it to make sure the view is loaded
            if (newView.loadView != undefined)
                newView.loadView();
            newView.x = leftToRight ? -root.width : root.width
            direction = leftToRight;
            previousView = currentView;
            currentView = newView;
            newView.opacity = 1;
            switchAnimation.start();
            if (instant) {
                switchAnimation.complete();
            }
        }
    }

    property variant switchAnimation :
        ParallelAnimation {
        NumberAnimation { target: previousView; property: "x"; easing.type: Easing.InOutSine
            to: direction ? root.width : -root.width; duration: container.duration }
        NumberAnimation { target: currentView; property: "x"; easing.type: Easing.InOutSine; to: 0; duration: container.duration  }


        onRunningChanged:  {
            if (!running && previousView) {
                previousView.opacity = 0;
                if (previousView.deactivationComplete != undefined) previousView.deactivationComplete();
                if (currentView.activationComplete != undefined) currentView.activationComplete();
            }
        }
    }
}
