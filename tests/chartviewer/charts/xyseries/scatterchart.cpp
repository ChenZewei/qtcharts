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

#include "charts.h"
#include "qchart.h"
#include "qscatterseries.h"

class ScatterChart: public Chart
{
public:
    QString name() { return QObject::tr("ScatterChart"); }
    QString category()  { return QObject::tr("XYSeries"); }
    QString subCategory() { return QString::null; }

    QChart *createChart(const DataTable &table)
    {
        QChart *chart = new QChart();
        chart->setTitle("Scatter chart");
        QString name("Series ");
        int nameIndex = 0;
        foreach (DataList list, table) {
            QScatterSeries *series = new QScatterSeries(chart);
            foreach (Data data, list)
                series->append(data.first);
            series->setName(name + QString::number(nameIndex));
            nameIndex++;
            chart->addSeries(series);
        }
        chart->createDefaultAxes();
        return chart;
    }
};

DECLARE_CHART(ScatterChart)
