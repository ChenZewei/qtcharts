/****************************************************************************
**
** Copyright (C) 2013 Digia Plc
** All rights reserved.
** For any questions to Digia, please use contact form at http://qt.digia.com
**
** This file is part of the Qt Commercial Charts Add-on.
**
** $QT_BEGIN_LICENSE$
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.
**
** If you have questions regarding the use of this file, please use
** contact form at http://qt.digia.com
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DECLARATIVEAXES_H
#define DECLARATIVEAXES_H

#include "qchartglobal.h"
#include <QObject>

QTCOMMERCIALCHART_BEGIN_NAMESPACE

class QAbstractAxis;

class DeclarativeAxes : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QAbstractAxis *axisX READ axisX WRITE setAxisX NOTIFY axisXChanged)
    Q_PROPERTY(QAbstractAxis *axisY READ axisY WRITE setAxisY NOTIFY axisYChanged)
    Q_PROPERTY(QAbstractAxis *axisXTop READ axisXTop WRITE setAxisXTop NOTIFY axisXTopChanged)
    Q_PROPERTY(QAbstractAxis *axisYRight READ axisYRight WRITE setAxisYRight NOTIFY axisYRightChanged)

public:
    explicit DeclarativeAxes(QObject *parent = 0);

    QAbstractAxis *axisX() { return m_axisX; }
    void setAxisX(QAbstractAxis *axis);
    QAbstractAxis *axisY() { return m_axisY; }
    void setAxisY(QAbstractAxis *axis);
    QAbstractAxis *axisXTop() { return m_axisXTop; }
    void setAxisXTop(QAbstractAxis *axis);
    QAbstractAxis *axisYRight() { return m_axisYRight; }
    void setAxisYRight(QAbstractAxis *axis);

public:
    void emitAxisXChanged() { emit axisXChanged(m_axisX); }
    void emitAxisYChanged() { emit axisYChanged(m_axisY); }
    void emitAxisXTopChanged() { emit axisXTopChanged(m_axisXTop); }
    void emitAxisYRightChanged() { emit axisYRightChanged(m_axisYRight); }

signals:
    void axisXChanged(QAbstractAxis *axis);
    void axisYChanged(QAbstractAxis *axis);
    void axisXTopChanged(QAbstractAxis *axis);
    void axisYRightChanged(QAbstractAxis *axis);

private:
    QAbstractAxis *m_axisX;
    QAbstractAxis *m_axisY;
    QAbstractAxis *m_axisXTop;
    QAbstractAxis *m_axisYRight;
};

QTCOMMERCIALCHART_END_NAMESPACE

#endif // DECLARATIVEAXES_H
