#ifndef FILTERSTA_H
#define FILTERSTA_H

#include "common.h"

#include <QDialog>
#include <QtCharts>

namespace Ui {
class FilterSta;
}

class FilterSta : public QDialog
{
    Q_OBJECT

public:
    explicit FilterSta(QList<_STATION> staList, double eewMag=0, QWidget *parent=nullptr);
    ~FilterSta();

    QList<_STATION> filteredStaVT;
    QList<_STATION> predStaList;

private:
    Ui::FilterSta *ui;

    QList<_STATION> oriStaVT;

    void makePGAvsDistGraph();
    QChartView *pgaVsDistView;
    QChart *pgaVsDistChart;

    QList<double> predVector;
    QList<double> predVectorUp;
    QList<double> predVectorDown;
};

#endif // FILTERSTA_H
