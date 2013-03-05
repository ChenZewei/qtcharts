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

#include "piechartitem_p.h"
#include "piesliceitem_p.h"
#include "qpieslice.h"
#include "qpieslice_p.h"
#include "qpieseries.h"
#include "qpieseries_p.h"
#include "chartpresenter_p.h"
#include "chartdataset_p.h"
#include "pieanimation_p.h"
#include <QPainter>
#include <QTimer>

QTCOMMERCIALCHART_BEGIN_NAMESPACE

PieChartItem::PieChartItem(QPieSeries *series, QGraphicsItem* item)
    : ChartItem(series->d_func(),item),
      m_series(series),
      m_animation(0)
{
    Q_ASSERT(series);

    QPieSeriesPrivate *p = QPieSeriesPrivate::fromSeries(series);
    connect(series, SIGNAL(visibleChanged()), this, SLOT(handleSeriesVisibleChanged()));
    connect(series, SIGNAL(opacityChanged()), this, SLOT(handleOpacityChanged()));
    connect(series, SIGNAL(added(QList<QPieSlice*>)), this, SLOT(handleSlicesAdded(QList<QPieSlice*>)));
    connect(series, SIGNAL(removed(QList<QPieSlice*>)), this, SLOT(handleSlicesRemoved(QList<QPieSlice*>)));
    connect(p, SIGNAL(horizontalPositionChanged()), this, SLOT(updateLayout()));
    connect(p, SIGNAL(verticalPositionChanged()), this, SLOT(updateLayout()));
    connect(p, SIGNAL(pieSizeChanged()), this, SLOT(updateLayout()));
    connect(p, SIGNAL(calculatedDataChanged()), this, SLOT(updateLayout()));

    // Note: the following does not affect as long as the item does not have anything to paint
    setZValue(ChartPresenter::PieSeriesZValue);

    // Note: will not create slice items until we have a proper rectangle to draw on.
}

PieChartItem::~PieChartItem()
{
    // slices deleted automatically through QGraphicsItem
    if (m_series) {
        m_series->disconnect(this);
        QPieSeriesPrivate::fromSeries(m_series)->disconnect(this);
    }
    foreach (QPieSlice *slice, m_sliceItems.keys()) {
        slice->disconnect(this);
        QPieSlicePrivate::fromSlice(slice)->disconnect(this);
    }
}

void PieChartItem::setAnimation(PieAnimation *animation)
{
    m_animation = animation;
}

ChartAnimation *PieChartItem::animation() const
{
    return m_animation;
}

void PieChartItem::handleDomainUpdated()
{
    QRectF rect(QPointF(0,0),domain()->size());
    if(m_rect!=rect){
        prepareGeometryChange();
        m_rect = rect;
        updateLayout();

        if (m_sliceItems.isEmpty())
              handleSlicesAdded(m_series->slices());
    }
}

void PieChartItem::updateLayout()
{
    // find pie center coordinates
    m_pieCenter.setX(m_rect.left() + (m_rect.width() * m_series->horizontalPosition()));
    m_pieCenter.setY(m_rect.top() + (m_rect.height() * m_series->verticalPosition()));

    // find maximum radius for pie
    m_pieRadius = m_rect.height() / 2;
    if (m_rect.width() < m_rect.height())
        m_pieRadius = m_rect.width() / 2;

    m_holeSize = m_pieRadius;
    // apply size factor
    m_pieRadius *= m_series->pieSize();
    m_holeSize *= m_series->holeSize();

    // set layouts for existing slice items
    foreach (QPieSlice *slice, m_series->slices()) {
        PieSliceItem *sliceItem = m_sliceItems.value(slice);
        if (sliceItem) {
            PieSliceData sliceData = updateSliceGeometry(slice);
            if (m_animation)
                presenter()->startAnimation(m_animation->updateValue(sliceItem, sliceData));
            else
                sliceItem->setLayout(sliceData);
        }
    }

    update();
}

void PieChartItem::handleSlicesAdded(QList<QPieSlice *> slices)
{
    // delay creating slice items until there is a proper rectangle
    if (!m_rect.isValid() && m_sliceItems.isEmpty())
        return;

    themeManager()->updateSeries(m_series);

    bool startupAnimation = m_sliceItems.isEmpty();

    foreach(QPieSlice * slice, slices) {
        PieSliceItem *sliceItem = new PieSliceItem(this);
        m_sliceItems.insert(slice, sliceItem);

        // Note: no need to connect to slice valueChanged() etc.
        // This is handled through calculatedDataChanged signal.
        connect(slice, SIGNAL(labelChanged()), this, SLOT(handleSliceChanged()));
        connect(slice, SIGNAL(labelVisibleChanged()), this, SLOT(handleSliceChanged()));
        connect(slice, SIGNAL(penChanged()), this, SLOT(handleSliceChanged()));
        connect(slice, SIGNAL(brushChanged()), this, SLOT(handleSliceChanged()));
        connect(slice, SIGNAL(labelBrushChanged()), this, SLOT(handleSliceChanged()));
        connect(slice, SIGNAL(labelFontChanged()), this, SLOT(handleSliceChanged()));

        QPieSlicePrivate *p = QPieSlicePrivate::fromSlice(slice);
        connect(p, SIGNAL(labelPositionChanged()), this, SLOT(handleSliceChanged()));
        connect(p, SIGNAL(explodedChanged()), this, SLOT(handleSliceChanged()));
        connect(p, SIGNAL(labelArmLengthFactorChanged()), this, SLOT(handleSliceChanged()));
        connect(p, SIGNAL(explodeDistanceFactorChanged()), this, SLOT(handleSliceChanged()));

        connect(sliceItem, SIGNAL(clicked(Qt::MouseButtons)), slice, SIGNAL(clicked()));
        connect(sliceItem, SIGNAL(hovered(bool)), slice, SIGNAL(hovered(bool)));

        PieSliceData sliceData = updateSliceGeometry(slice);
        if (m_animation)
            presenter()->startAnimation(m_animation->addSlice(sliceItem, sliceData, startupAnimation));
        else
            sliceItem->setLayout(sliceData);
    }
}

void PieChartItem::handleSlicesRemoved(QList<QPieSlice *> slices)
{
    themeManager()->updateSeries(m_series);

    foreach (QPieSlice *slice, slices) {

        PieSliceItem *sliceItem = m_sliceItems.value(slice);

        // this can happen if you call append() & remove() in a row so that PieSliceItem is not even created
        if (!sliceItem)
            continue;

        m_sliceItems.remove(slice);
        slice->disconnect(this);
        QPieSlicePrivate::fromSlice(slice)->disconnect(this);

        if (m_animation)
            presenter()->startAnimation(m_animation->removeSlice(sliceItem)); // animator deletes the PieSliceItem
        else
            delete sliceItem;
    }
}

void PieChartItem::handleSliceChanged()
{
    QPieSlice *slice = qobject_cast<QPieSlice *>(sender());
    if (!slice) {
        QPieSlicePrivate *slicep = qobject_cast<QPieSlicePrivate *>(sender());
        slice = slicep->q_ptr;
    }
    Q_ASSERT(m_sliceItems.contains(slice));

    PieSliceItem *sliceItem = m_sliceItems.value(slice);
    PieSliceData sliceData = updateSliceGeometry(slice);
    if (m_animation)
        presenter()->startAnimation(m_animation->updateValue(sliceItem, sliceData));
    else
        sliceItem->setLayout(sliceData);

    update();
}

void PieChartItem::handleSeriesVisibleChanged()
{
    setVisible(m_series->isVisible());
}

void PieChartItem::handleOpacityChanged()
{
    setOpacity(m_series->opacity());
}

PieSliceData PieChartItem::updateSliceGeometry(QPieSlice *slice)
{
    PieSliceData &sliceData = QPieSlicePrivate::fromSlice(slice)->m_data;
    sliceData.m_center = PieSliceItem::sliceCenter(m_pieCenter, m_pieRadius, slice);
    sliceData.m_radius = m_pieRadius;
    sliceData.m_holeRadius = m_holeSize;
    return sliceData;
}

#include "moc_piechartitem_p.cpp"

QTCOMMERCIALCHART_END_NAMESPACE
