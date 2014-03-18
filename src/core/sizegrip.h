#ifndef SIZEGRIP_H
#define SIZEGRIP_H

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

#include <qglobal.h>
#if QT_VERSION >= 0x050000
// Qt5 code
#   include <QtWidgets>
#else
// Qt4 code
#   include <QtCore>
#   include <QtGui>
#endif

class CSizeGrip : public QWidget
{
    Q_OBJECT
public:
    explicit CSizeGrip(QWidget *parent,QString image);
    ~CSizeGrip();

    QSize sizeHint() const;
    void setVisible(bool);

    QImage sizeImage;

    QWidget * mainWidget;
protected:
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *mouseEvent);
    void moveEvent(QMoveEvent *moveEvent);
    void showEvent(QShowEvent *showEvent);
    void hideEvent(QHideEvent *hideEvent);
    bool eventFilter(QObject *, QEvent *);
    bool event(QEvent *);
#ifdef Q_WS_WIN
    bool winEvent(MSG *m, long *result);
#endif

public:
    void init();
    QPoint p;
    QRect r;
    int d;
    int dxMax;
    int dyMax;
    Qt::Corner m_corner;
    bool gotMousePress;
    QWidget *tlw;
#ifdef Q_WS_MAC
    void updateMacSizer(bool hide) const;
#endif
    Qt::Corner corner() const;
    inline bool atBottom() const
    {
        return m_corner == Qt::BottomRightCorner || m_corner == Qt::BottomLeftCorner;
    }

    inline bool atLeft() const
    {
        return m_corner == Qt::BottomLeftCorner || m_corner == Qt::TopLeftCorner;
    }

    QWidget *qt_sizegrip_topLevelWidget()
    {
        return mainWidget;
//        QWidget *w = fw->parentWidget();
//        while (w && !w->isWidgetType() && w->windowType() != Qt::SubWindow)
//            w = w->parentWidget();
//        return w;
    }

    void updateTopLevelWidget()
    {
        QWidget *w = qt_sizegrip_topLevelWidget();
        if (tlw == w)
            return;
        if (tlw)
            tlw->removeEventFilter(this);
        tlw = w;
        if (tlw)
            tlw->installEventFilter(this);
    }

    // This slot is invoked by QLayout when the size grip is added to
    // a layout or reparented after the tlw is shown. This re-implementation is basically
    // the same as QWidgetPrivate::_q_showIfNotHidden except that it checks
    // for Qt::WindowFullScreen and Qt::WindowMaximized as well.
    void _q_showIfNotHidden()
    {
        bool showSizeGrip = !(isHidden() &&testAttribute(Qt::WA_WState_ExplicitShowHide));
        updateTopLevelWidget();
        if (tlw && showSizeGrip) {
            Qt::WindowStates sizeGripNotVisibleState = Qt::WindowFullScreen;
#ifndef Q_WS_MAC
            sizeGripNotVisibleState |= Qt::WindowMaximized;
#endif
            // Don't show the size grip if the tlw is maximized or in full screen mode.
            showSizeGrip = !(tlw->windowState() & sizeGripNotVisibleState);
        }
        if (showSizeGrip)
            setVisible(true);
    }
private:
//    Q_PRIVATE_SLOT(d_func(), void _q_showIfNotHidden())
};

#endif // SIZEGRIP_H
