// from http://www.evileg.ru/baza-znanij/qt-qml-android/navigation-drawer-v-qt-qml-android.html


import QtQuick 2.5
import QtQuick.Window 2.0

Rectangle {
    id: panel

    // Пересчёт независимых от плотности пикселей в физические пиксели устройства
    readonly property int dpi: Screen.pixelDensity * 25.4
    function dp(x){ return (dpi < 120) ? x : x*(dpi/160); }

    // Свойства Navigation Drawer
    property bool open: false           // Состояние Navigation Drawer - Открыт/Закрыт
    property int position: Qt.LeftEdge  // Положение Navigation Drawer - Слева/Справа

    // Функции открытия и закрытия Navigation Drawer
    function show() { open = true; }
    function hide() { open = false; }
    function toggle() { open = open ? false : true; }

    // Внутренние свойства Navigation Drawer
    readonly property bool _rightEdge: position === Qt.RightEdge
    readonly property int _closeX: _rightEdge ? _rootItem.width : - panel.width
    readonly property int _openX: _rightEdge ? _rootItem.width - width : 0
    readonly property int _minimumX: _rightEdge ? _rootItem.width - panel.width : -panel.width
    readonly property int _maximumX: _rightEdge ? _rootItem.width : 0
    readonly property int _pullThreshold: panel.width/2
    readonly property int _slideDuration: 260
    readonly property int _openMarginSize: dp(20)

    property real _velocity: 0
    property real _oldMouseX: -1

    property Item _rootItem: parent

    on_RightEdgeChanged: _setupAnchors()
    onOpenChanged: completeSlideDirection()

    width: (Screen.width > Screen.height) ? dp(320) : Screen.width - dp(56)
    height: parent.height
    x: _closeX
    z: 10

    function _setupAnchors() {
        _rootItem = parent;

        shadow.anchors.right = undefined;
        shadow.anchors.left = undefined;

        mouse.anchors.left = undefined;
        mouse.anchors.right = undefined;

        if (_rightEdge) {
            mouse.anchors.right = mouse.parent.right;
            shadow.anchors.right = panel.left;
        } else {
            mouse.anchors.left = mouse.parent.left;
            shadow.anchors.left = panel.right;
        }

        slideAnimation.enabled = false;
        panel.x = _rightEdge ? _rootItem.width :  - panel.width;
        slideAnimation.enabled = true;
    }

    function completeSlideDirection() {
        if (open) {
            panel.x = _openX;
        } else {
            panel.x = _closeX;
            Qt.inputMethod.hide();
        }
    }

    function handleRelease() {
        var velocityThreshold = dp(5)
        if ((_rightEdge && _velocity > velocityThreshold) ||
                (!_rightEdge && _velocity < -velocityThreshold)) {
            panel.open = false;
            completeSlideDirection()
        } else if ((_rightEdge && _velocity < -velocityThreshold) ||
                   (!_rightEdge && _velocity > velocityThreshold)) {
            panel.open = true;
            completeSlideDirection()
        } else if ((_rightEdge && panel.x < _openX + _pullThreshold) ||
                   (!_rightEdge && panel.x > _openX - _pullThreshold) ) {
            panel.open = true;
            panel.x = _openX;
        } else {
            panel.open = false;
            panel.x = _closeX;
        }
    }

    function handleClick(mouse) {
        if ((_rightEdge && mouse.x < panel.x ) || mouse.x > panel.width) {
            open = false;
        }
    }

    onPositionChanged: {
        if (!(position === Qt.RightEdge || position === Qt.LeftEdge )) {
            console.warn("SlidePanel: Unsupported position.")
        }
    }

    Behavior on x {
        id: slideAnimation
        enabled: !mouse.drag.active
        NumberAnimation {
            duration: _slideDuration
            easing.type: Easing.OutCubic
        }
    }

    NumberAnimation on x {
        id: holdAnimation
        to: _closeX + (_openMarginSize * (_rightEdge ? -1 : 1))
        running : false
        easing.type: Easing.OutCubic
        duration: 200
    }

    MouseArea {
        id: mouse
        parent: _rootItem

        y: _rootItem.y
        width: open ? _rootItem.width : _openMarginSize
        height: _rootItem.height
        onPressed:  if (!open) holdAnimation.restart();
        onClicked: handleClick(mouse)
        drag.target: panel
        drag.minimumX: _minimumX
        drag.maximumX: _maximumX
        drag.axis: Qt.Horizontal
        drag.onActiveChanged: if (active) holdAnimation.stop()
        onReleased: handleRelease()
        z: open ? 1 : 0
        onMouseXChanged: {
            _velocity = (mouse.x - _oldMouseX);
            _oldMouseX = mouse.x;
        }
    }

    Connections {
        target: _rootItem
        onWidthChanged: {
            slideAnimation.enabled = false
            panel.completeSlideDirection()
            slideAnimation.enabled = true
        }
    }

    Rectangle {
        id: backgroundBlackout
        parent: _rootItem
        anchors.fill: parent
        opacity: 0.5 * Math.min(1, Math.abs(panel.x - _closeX) / _rootItem.width/2)
        color: "black"
    }

    Item {
        id: shadow
        anchors.left: panel.right
        anchors.leftMargin: _rightEdge ? 0 : dp(10)
        height: parent.height

        Rectangle {
            height: dp(10)
            width: panel.height
            rotation: 90
            opacity: Math.min(1, Math.abs(panel.x - _closeX)/ _openMarginSize)
            transformOrigin: Item.TopLeft
            gradient: Gradient{
                GradientStop { position: _rightEdge ? 1 : 0 ; color: "#00000000"}
                GradientStop { position: _rightEdge ? 0 : 1 ; color: "#2c000000"}
            }
        }
    }
}
