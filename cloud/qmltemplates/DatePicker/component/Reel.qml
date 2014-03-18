import QtQuick 1.0

Rectangle {
    id: reel    

    property alias interactive: path.interactive
    property int index: 0
    property bool moving: false
    property ListModel model    
    property Component delegate
    property int itemsShown: 4
    property bool autoClose: true
    property alias closingDelay: clippingTimer.interval
    property int closingTimeout: 1000

    function open() { focus = true; clip = false }
    function close() { clip = true }
    function toggle() { clip ? open() : close() }
    function shiftZ(obj, delta) {
        if (typeof obj.z != 'undefined') obj.z += delta
        if (obj.parent) shiftZ(obj.parent, delta) // Set z recursively to parent
    }

    width: 100
    height: 100
    color: "transparent"
    clip: true
    // Close reel when the focus is lost
    onFocusChanged: if (!focus) close()
    // Bring to front if not clipped
    onClipChanged:  { clip ? shiftZ(reel, -500) : shiftZ(reel, 500) }

    onIndexChanged: path.currentIndex = reel.index

    PathView {
        id: path
        width: parent.width
        height: (pathItemCount-1)*parent.height
        pathItemCount: parent.itemsShown+1
        clip: true
        anchors.centerIn: parent
        model: parent.model
        delegate: reel.delegate

        preferredHighlightBegin: 0.5
        preferredHighlightEnd: 0.5
        highlightRangeMode: PathView.StrictlyEnforceRange
        focus: false

        path: Path {
            startX: path.x+path.width/2; startY: 1-reel.height/2
            PathLine {x: path.x+path.width/2; y: path.height+reel.height/2-1}
        }
        onMovementStarted: { reel.moving = true; clippingTimer.stop(); reel.open()}
        onMovementEnded: {
            if (reel.autoClose) {
                clippingTimer.restart();
            }
            reel.index = path.currentIndex;
            reel.moving = false;
        }

        Timer {
            id: clippingTimer
            repeat: false; interval: reel.closingTimeout;
            triggeredOnStart: false; onTriggered: reel.close()
        }        
    }
}
