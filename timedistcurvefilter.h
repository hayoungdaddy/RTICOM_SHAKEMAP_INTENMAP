#ifndef TIMEDISTCURVEFILTER_H
#define TIMEDISTCURVEFILTER_H

#include "common.h"

#include <QDialog>
#include <QtCharts>

namespace Ui {
class TimeDistCurveFilter;
}

class TimeDistCurveFilter : public QDialog
{
    Q_OBJECT

public:
    explicit TimeDistCurveFilter(QList<_STATION> staList, int eewTime=0, QWidget *parent = nullptr);
    ~TimeDistCurveFilter();

    QList<_STATION> resultStaList;

private:
    Ui::TimeDistCurveFilter *ui;

    QList<_STATION> oriStaList;
    int eqTime;

    void makeGraphMINMAXVelRange();
    QChartView *eqChartView1;
    QChart *eqChart1;
    QList<_STATION> resultStaListLevel1;
    double slope, intercept, coeff;

    void makeGraphlr();
    QChartView *eqChartView2;
    QChart *eqChart2;
    QList<_STATION> resultStaListLevel2;

    /*
    void makeEQGraph2();
    QChartView *eqChartView2;
    QChart *eqChart2;
    QVector<_STATION> outOfRangeForMAXMINVelStaV;
    QVector<_STATION> overDistStaV;
    double slope, intercept, coeff;

    void makeEQGraph3();
    QChartView *eqChartView3;
    QChart *eqChart3;
    QVector<_STATION> overSDStaV;
    */
};

#endif // TIMEDISTCURVEFILTER_H
