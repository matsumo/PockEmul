/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include <QDesktopWidget>
#if QT_VERSION >= 0x050000
// Qt5 code
#   include <QtWidgets>
#else
// Qt4 code
#   include <QtCore>
#   include <QtGui>
#   include <QWidget>
#endif
#include <QAbstractScrollArea>

#include "sizegrip.h"

#include "pobject.h"

//#include "qapplication.h"
//#include "qevent.h"
//#include "qpainter.h"
//#include "qstyle.h"
//#include "qstyleoption.h"
//#include "qlayout.h"
//#include "qdebug.h"


#if defined(Q_WS_X11)
//#include <private/qt_x11_p.h>
#elif defined (Q_WS_WIN)
#include "qt_windows.h"
#endif
#ifdef Q_WS_MAC
#include <private/qt_mac_p.h>
#endif


#define SZ_SIZEBOTTOMRIGHT  0xf008
#define SZ_SIZEBOTTOMLEFT   0xf007
#define SZ_SIZETOPLEFT      0xf004
#define SZ_SIZETOPRIGHT     0xf005



#ifdef Q_WS_MAC
void CSizeGrip::updateMacSizer(bool hide) const
{

    if (QApplication::closingDown() || !parent)
        return;
    QWidget *topLevelWindow = qt_sizegrip_topLevelWidget(const_cast<CSizeGrip *>(this));
    if(topLevelWindow && topLevelWindow->isWindow())
        QWidget::qt_mac_update_sizer(topLevelWindow, hide ? -1 : 1);
}
#endif

Qt::Corner CSizeGrip::corner() const
{

    QWidget *tlw = mainWidget;//qt_sizegrip_topLevelWidget();
    const QPoint sizeGripPos = mapTo(tlw, QPoint(0, 0));
    bool isAtBottom = sizeGripPos.y() >= tlw->height() / 2;
    bool isAtLeft = sizeGripPos.x() <= tlw->width() / 2;
    if (isAtLeft)
        return isAtBottom ? Qt::BottomLeftCorner : Qt::TopLeftCorner;
    else
        return isAtBottom ? Qt::BottomRightCorner : Qt::TopRightCorner;
}

/*!
    \class CSizeGrip

    \brief The CSizeGrip class provides a resize handle for resizing top-level windows.

    \ingroup mainwindow-classes
    \ingroup basicwidgets

    This widget works like the standard Windows resize handle. In the
    X11 version this resize handle generally works differently from
    the one provided by the system if the X11 window manager does not
    support necessary modern post-ICCCM specifications.

    Put this widget anywhere in a widget tree and the user can use it
    to resize the top-level window or any widget with the Qt::SubWindow
    flag set. Generally, this should be in the lower right-hand corner.
    Note that QStatusBar already uses this widget, so if you have a
    status bar (e.g., you are using QMainWindow), then you don't need
    to use this widget explicitly.

    On some platforms the size grip automatically hides itself when the
    window is shown full screen or maximised.

    \table 50%
    \row \o \inlineimage plastique-sizegrip.png Screenshot of a Plastique style size grip
    \o A size grip widget at the bottom-right corner of a main window, shown in the
    \l{Plastique Style Widget Gallery}{Plastique widget style}.
    \endtable

    The CSizeGrip class inherits QWidget and reimplements the \l
    {QWidget::mousePressEvent()}{mousePressEvent()} and \l
    {QWidget::mouseMoveEvent()}{mouseMoveEvent()} functions to feature
    the resize functionality, and the \l
    {QWidget::paintEvent()}{paintEvent()} function to render the
    size grip widget.

    \sa QStatusBar QWidget::windowState()
*/


/*!
    Constructs a resize corner as a child widget of  the given \a
    parent.
*/
CSizeGrip::CSizeGrip(QWidget * parent, QString image)
    : QWidget(parent, 0)
{
    mainWidget = parent;
    init();
    sizeImage = QImage(image).scaled(16,16);
}


void CSizeGrip::init()
{

    dxMax = 0;
    dyMax = 0;
    tlw = 0;
    m_corner = isLeftToRight() ? Qt::BottomRightCorner : Qt::BottomLeftCorner;
    gotMousePress = false;

#if !defined(QT_NO_CURSOR) && !defined(Q_WS_MAC)
    setCursor(m_corner == Qt::TopLeftCorner || m_corner == Qt::BottomRightCorner
                 ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
#endif
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
    updateTopLevelWidget();
}


/*!
    Destroys this size grip.
*/
CSizeGrip::~CSizeGrip()
{
}

/*!
  \reimp
*/
QSize CSizeGrip::sizeHint() const
{
    QStyleOption opt(0);
    opt.init(this);
    return (style()->sizeFromContents(QStyle::CT_SizeGrip, &opt, QSize(16, 16), this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    Paints the resize grip.

    Resize grips are usually rendered as small diagonal textured lines
    in the lower-right corner. The paint event is passed in the \a
    event parameter.
*/
void CSizeGrip::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
#if 0
    QStyleOptionSizeGrip opt;
    opt.init(this);
    opt.corner = m_corner;
    style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
#else
//    qWarning()<<"w:"<<width()<<" h:"<<height();
    painter.drawImage(0,0,sizeImage);
#endif
}

/*!
    \fn void CSizeGrip::mousePressEvent(QMouseEvent * event)

    Receives the mouse press events for the widget, and primes the
    resize operation. The mouse press event is passed in the \a event
    parameter.
*/
void CSizeGrip::mousePressEvent(QMouseEvent * e)
{
    ungrabGesture(Qt::TapAndHoldGesture);

    if (e->button() != Qt::LeftButton) {
        QWidget::mousePressEvent(e);
        return;
    }


    QWidget *tlw = qt_sizegrip_topLevelWidget();
    p = e->globalPos();
    gotMousePress = true;
    r = tlw->geometry();

#ifdef Q_WS_X11
    #if QT_VERSION >= 0x050000
    // Use a native X11 sizegrip for "real" top-level windows if supported.
    if (tlw->isWindow() && X11->isSupportedByWM(ATOM(_NET_WM_MOVERESIZE))
        && !(tlw->windowFlags() & Qt::X11BypassWindowManagerHint)
        && !tlw->testAttribute(Qt::WA_DontShowOnScreen) && !qt_widget_private(tlw)->hasHeightForWidth()) {
        XEvent xev;
        xev.xclient.type = ClientMessage;
        xev.xclient.message_type = ATOM(_NET_WM_MOVERESIZE);
        xev.xclient.display = X11->display;
        xev.xclient.window = tlw->winId();
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = e->globalPos().x();
        xev.xclient.data.l[1] = e->globalPos().y();
        if (atBottom())
            xev.xclient.data.l[2] = atLeft() ? 6 : 4; // bottomleft/bottomright
        else
            xev.xclient.data.l[2] = atLeft() ? 0 : 2; // topleft/topright
        xev.xclient.data.l[3] = Button1;
        xev.xclient.data.l[4] = 0;
        XUngrabPointer(X11->display, X11->time);
        XSendEvent(X11->display, QX11Info::appRootWindow(x11Info().screen()), False,
                   SubstructureRedirectMask | SubstructureNotifyMask, &xev);
        return;
    }
#endif
#endif // Q_WS_X11
#ifdef Q_WS_WIN
    #if QT_VERSION >= 0x050000
    if (tlw->isWindow() && !tlw->testAttribute(Qt::WA_DontShowOnScreen) && !qt_widget_private(tlw)->hasHeightForWidth()) {
        uint orientation = 0;
        if (atBottom())
            orientation = atLeft() ? SZ_SIZEBOTTOMLEFT : SZ_SIZEBOTTOMRIGHT;
        else
            orientation = atLeft() ? SZ_SIZETOPLEFT : SZ_SIZETOPRIGHT;

        ReleaseCapture();
        PostMessage(tlw->winId(), WM_SYSCOMMAND, orientation, 0);
        return;
    }
#endif
#endif // Q_WS_WIN

    // Find available desktop/workspace geometry.
    QRect availableGeometry;
    bool hasVerticalSizeConstraint = true;
    bool hasHorizontalSizeConstraint = true;
    if (tlw->isWindow())
        availableGeometry = QApplication::desktop()->availableGeometry(tlw);
    else {
        const QWidget *tlwParent = tlw->parentWidget();
        // Check if tlw is inside QAbstractScrollArea/QScrollArea.
        // If that's the case tlw->parentWidget() will return the viewport
        // and tlw->parentWidget()->parentWidget() will return the scroll area.
#ifndef QT_NO_SCROLLAREA
        QAbstractScrollArea *scrollArea = qobject_cast<QAbstractScrollArea *>(tlwParent->parentWidget());
        if (scrollArea) {
            hasHorizontalSizeConstraint = scrollArea->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
            hasVerticalSizeConstraint = scrollArea->verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff;
        }
#endif // QT_NO_SCROLLAREA
        availableGeometry = tlwParent->contentsRect();
    }

    // Find frame geometries, title bar height, and decoration sizes.
    const QRect frameGeometry = tlw->frameGeometry();
    const int titleBarHeight = qMax(tlw->geometry().y() - frameGeometry.y(), 0);
    const int bottomDecoration = qMax(frameGeometry.height() - tlw->height() - titleBarHeight, 0);
    const int leftRightDecoration = qMax((frameGeometry.width() - tlw->width()) / 2, 0);

    // Determine dyMax depending on whether the sizegrip is at the bottom
    // of the widget or not.
    if (atBottom()) {
        if (hasVerticalSizeConstraint)
            dyMax = availableGeometry.bottom() - r.bottom() - bottomDecoration;
        else
            dyMax = INT_MAX;
    } else {
        if (hasVerticalSizeConstraint)
            dyMax = availableGeometry.y() - r.y() + titleBarHeight;
        else
            dyMax = -INT_MAX;
    }

    // In RTL mode, the size grip is to the left; find dxMax from the desktop/workspace
    // geometry, the size grip geometry and the width of the decoration.
    if (atLeft()) {
        if (hasHorizontalSizeConstraint)
            dxMax = availableGeometry.x() - r.x() + leftRightDecoration;
        else
            dxMax = -INT_MAX;
    } else {
        if (hasHorizontalSizeConstraint)
            dxMax = availableGeometry.right() - r.right() - leftRightDecoration;
        else
            dxMax = INT_MAX;
    }
}


/*!
    \fn void CSizeGrip::mouseMoveEvent(QMouseEvent * event)
    Resizes the top-level widget containing this widget. The mouse
    move event is passed in the \a event parameter.
*/
void CSizeGrip::mouseMoveEvent(QMouseEvent * e)
{
    if (e->buttons() != Qt::LeftButton) {
        QWidget::mouseMoveEvent(e);
        return;
    }


    QWidget* tlw = qt_sizegrip_topLevelWidget();
    if (!gotMousePress || tlw->testAttribute(Qt::WA_WState_ConfigPending))
        return;

#ifdef Q_WS_X11
    #if QT_VERSION >= 0x050000
    if (tlw->isWindow() && X11->isSupportedByWM(ATOM(_NET_WM_MOVERESIZE))
        && tlw->isTopLevel() && !(tlw->windowFlags() & Qt::X11BypassWindowManagerHint)
        && !tlw->testAttribute(Qt::WA_DontShowOnScreen) && !qt_widget_private(tlw)->hasHeightForWidth())
        return;
#endif
#endif
#ifdef Q_WS_WIN
    #if QT_VERSION >= 0x050000
    if (tlw->isWindow() && GetSystemMenu(tlw->winId(), FALSE) != 0 && internalWinId()
        && !tlw->testAttribute(Qt::WA_DontShowOnScreen) && !qt_widget_private(tlw)->hasHeightForWidth()) {
        MSG msg;
        while(PeekMessage(&msg, winId(), WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE));
        return;
    }
#endif
#endif

    QPoint np(e->globalPos());

    // Don't extend beyond the available geometry; bound to dyMax and dxMax.
    QSize ns;
    if (atBottom())
        ns.rheight() = r.height() + qMin(np.y() - p.y(), dyMax);
    else
        ns.rheight() = r.height() - qMax(np.y() - p.y(), dyMax);

    if (atLeft())
        ns.rwidth() = r.width() - qMax(np.x() - p.x(), dxMax);
    else
        ns.rwidth() = r.width() + qMin(np.x() - p.x(), dxMax);

    ns = QLayout::closestAcceptableSize(tlw, ns);

    QPoint p;
    QRect nr(p, ns);
    if (atBottom()) {
        if (atLeft())
            nr.moveTopRight(r.topRight());
        else
            nr.moveTopLeft(r.topLeft());
    } else {
        if (atLeft())
            nr.moveBottomRight(r.bottomRight());
        else
            nr.moveBottomLeft(r.bottomLeft());
    }

    ((CPObject*)tlw)->changeGeometrySize(nr.x(),nr.y(),nr.width(),nr.height());
}

/*!
  \reimp
*/
void CSizeGrip::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    grabGesture(Qt::TapAndHoldGesture);

    if (mouseEvent->button() == Qt::LeftButton) {

        gotMousePress = false;
        p = QPoint();
    } else {
        QWidget::mouseReleaseEvent(mouseEvent);
    }
}

/*!
  \reimp
*/
void CSizeGrip::moveEvent(QMoveEvent * /*moveEvent*/)
{

    // We're inside a resize operation; no update necessary.
    if (!p.isNull())
        return;

    m_corner = corner();
#if !defined(QT_NO_CURSOR) && !defined(Q_WS_MAC)
    setCursor(m_corner == Qt::TopLeftCorner || m_corner == Qt::BottomRightCorner
              ? Qt::SizeFDiagCursor : Qt::SizeBDiagCursor);
#endif
}

/*!
  \reimp
*/
void CSizeGrip::showEvent(QShowEvent *showEvent)
{
#ifdef Q_WS_MAC
    d_func()->updateMacSizer(false);
#endif
    QWidget::showEvent(showEvent);
}

/*!
  \reimp
*/
void CSizeGrip::hideEvent(QHideEvent *hideEvent)
{
#ifdef Q_WS_MAC
    d_func()->updateMacSizer(true);
#endif
    QWidget::hideEvent(hideEvent);
}

/*!
    \reimp
*/
void CSizeGrip::setVisible(bool visible)
{
    QWidget::setVisible(visible);
}

/*! \reimp */
bool CSizeGrip::eventFilter(QObject *o, QEvent *e)
{

    if ((isHidden() && testAttribute(Qt::WA_WState_ExplicitShowHide))
        || e->type() != QEvent::WindowStateChange
        || o != tlw) {
        return QWidget::eventFilter(o, e);
    }
    Qt::WindowStates sizeGripNotVisibleState = Qt::WindowFullScreen;
#ifndef Q_WS_MAC
    sizeGripNotVisibleState |= Qt::WindowMaximized;
#endif
    // Don't show the size grip if the tlw is maximized or in full screen mode.
    setVisible(!(tlw->windowState() & sizeGripNotVisibleState));
    setAttribute(Qt::WA_WState_ExplicitShowHide, false);
    return QWidget::eventFilter(o, e);
}

/*!
    \reimp
*/
bool CSizeGrip::event(QEvent *event)
{
    return QWidget::event(event);
}

#ifdef Q_WS_WIN
/*! \reimp */
bool CSizeGrip::winEvent( MSG *m, long *result )
{
    return QWidget::winEvent(m, result);
}
#endif

