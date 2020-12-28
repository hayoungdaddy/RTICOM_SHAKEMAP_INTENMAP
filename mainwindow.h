#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"
#include "timedistcurvefilter.h"
#include "filtersta.h"

#include <QMainWindow>
#include <QProgressDialog>
#include <QtConcurrent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::MainWindow *ui;

    void paint(QPainter *painter,  QPaintEvent *event, int type);

    TimeDistCurveFilter *timedistcurvefilter;
    FilterSta *gmpefiltersta;
    FilterSta *allfiltersta;

    _EVENT event;
    QList<_STATION> oriStaList;
    QList<_STATION> timedistCurveFilterStaList;
    QList<_STATION> gmpeFilteredStaList;
    QList<_STATION> allFilteredStaList;
    QList<_STATION> predStaList;

    QList<_POINT> oriPoints;
    QList<_POINT> timedistCurveFilterPoints;
    QList<_POINT> gmpePoints;
    QList<_POINT> allFilteredPoints;
    QList<_POINT> predPoints;

    bool isFilterLevel1, isFilterLevel2;
    bool isPredict;

    void readEventFile(QString);    
    QList<_POINT> processShakeMap(QList<_STATION>, QList<_POINT>);
    void displayRTICOM();
    void displayShakeMap();
    void displayIntenMap();

    /* ShakeMap */
    void initProj();
    projPJ pj_eqc ;
    projPJ pj_longlat ;
    int  ll2xy(projPJ src, projPJ target, float lon, float lat, int *x, int *y) ;
    QList<_POINT> init_MSM(QList<_STATION>, int);

    int originX, originY;
    int oriMaxX, oriMaxY;
    int timeDistCurveMaxX, timeDistCurveMaxY;
    int gmpeMaxX, gmpeMaxY;
    int allMaxX, allMaxY;
    int predMaxX, predMaxY;

private slots:
    void filter1Changed(int);
    void showFilter1();
    void filter2Changed(int);
    void showFilter2();
    void showPredict(int);
};

#endif // MAINWINDOW_H
