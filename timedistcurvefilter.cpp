#include "timedistcurvefilter.h"
#include "ui_timedistcurvefilter.h"

TimeDistCurveFilter::TimeDistCurveFilter(QList<_STATION> staList, int eewTime, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TimeDistCurveFilter)
{
    ui->setupUi(this);

    oriStaList = staList;
    eqTime = eewTime;

    eqChartView1 = new QChartView();
    eqChart1 = new QChart();
    eqChartView2 = new QChartView();
    eqChart2 = new QChart();

    makeGraphMINMAXVelRange();
    makeGraphlr();
    resultStaList = resultStaListLevel2;

    ui->nTotal->setText(QString::number(staList.size()));
}

TimeDistCurveFilter::~TimeDistCurveFilter()
{
    delete ui;
}

void TimeDistCurveFilter::makeGraphlr()
{
    QScatterSeries *liveDataSeries = new QScatterSeries();
    QScatterSeries *deadDataSeries = new QScatterSeries();
    QLineSeries *lrSeries = new QLineSeries();
    QLineSeries *lrUpSeries = new QLineSeries();
    QLineSeries *lrDownSeries = new QLineSeries();
    lrSeries->setName("Linear Regression");
    lrUpSeries->setName("Linear Regression(+" + QString::number(THRESHOLD_FOR_SECOND) + "sec)");
    lrDownSeries->setName("Linear Regression(-" + QString::number(THRESHOLD_FOR_SECOND) + "sec)");

    for(int i=0;i<resultStaListLevel1.size();i++)
    {
        int difftime = resultStaListLevel1.at(i).maxPGATime[CHAN] - eqTime;

        double value, minValue, maxValue;
        value = (resultStaListLevel1.at(i).distance*slope) + intercept; // real y
        minValue = value - THRESHOLD_FOR_SECOND;
        maxValue = value + THRESHOLD_FOR_SECOND;

        if((double)difftime >= minValue && (double)difftime <= maxValue)
        {
            liveDataSeries->append(resultStaListLevel1.at(i).distance, difftime);
            resultStaListLevel2.push_back(resultStaListLevel1.at(i));
        }
        else
            deadDataSeries->append(resultStaListLevel1.at(i).distance, difftime);
    }

    ui->nError->setText(QString::number(ui->nError->text().toInt() + deadDataSeries->count()));
    ui->nUsed->setText(QString::number(liveDataSeries->count()));

    lrSeries->append(1, (slope*1)+intercept);
    lrSeries->append(THRESHOLD_FOR_DIST, (slope*THRESHOLD_FOR_DIST)+intercept);
    lrUpSeries->append(1, (slope*1)+intercept+THRESHOLD_FOR_SECOND);
    lrUpSeries->append(THRESHOLD_FOR_DIST, (slope*THRESHOLD_FOR_DIST)+intercept+THRESHOLD_FOR_SECOND);
    lrDownSeries->append(1, (slope*1)+intercept-THRESHOLD_FOR_SECOND);
    lrDownSeries->append(THRESHOLD_FOR_DIST, (slope*THRESHOLD_FOR_DIST)+intercept-THRESHOLD_FOR_SECOND);

    eqChart2->addSeries(liveDataSeries);
    eqChart2->addSeries(deadDataSeries);
    eqChart2->addSeries(lrSeries);
    eqChart2->addSeries(lrUpSeries);
    eqChart2->addSeries(lrDownSeries);

    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    axisX->setTitleText("DISTANCE (Km)");
    axisY->setTitleText("TIME (s)");
    axisX->setLabelFormat("%i");
    axisY->setLabelFormat("%i");

    axisX->setRange(0, THRESHOLD_FOR_DIST);
    axisY->setRange(0, (slope*THRESHOLD_FOR_DIST)+intercept+THRESHOLD_FOR_SECOND);
    axisX->setTickCount(THRESHOLD_FOR_DIST/50 + 1);
    axisY->setTickCount(((slope*THRESHOLD_FOR_DIST)+intercept+THRESHOLD_FOR_SECOND)/20 + 1);

    eqChart2->addAxis(axisX, Qt::AlignBottom);
    eqChart2->addAxis(axisY, Qt::AlignLeft);
    liveDataSeries->attachAxis(axisX);
    liveDataSeries->attachAxis(axisY);
    deadDataSeries->attachAxis(axisX);
    deadDataSeries->attachAxis(axisY);
    lrSeries->attachAxis(axisX);
    lrSeries->attachAxis(axisY);
    lrUpSeries->attachAxis(axisX);
    lrUpSeries->attachAxis(axisY);
    lrDownSeries->attachAxis(axisX);
    lrDownSeries->attachAxis(axisY);

    eqChartView2->setRenderHint(QPainter::Antialiasing);
    eqChartView2->setChart(eqChart2);
    ui->graph2LO->addWidget(eqChartView2);
}

void TimeDistCurveFilter::makeGraphMINMAXVelRange()
{
    QScatterSeries *liveDataSeries = new QScatterSeries();
    QScatterSeries *deadDataSeries = new QScatterSeries();
    QLineSeries *maxWaveSeries = new QLineSeries();
    QLineSeries *minWaveSeries = new QLineSeries();
    QLineSeries *lrSeries = new QLineSeries();
    liveDataSeries->setName("Lived Station");
    deadDataSeries->setName("Out of bound");
    maxWaveSeries->setName("MAX Velocity (" + QString::number(MAX_VELOCITY, 'f', 1) + "Km/s)");
    minWaveSeries->setName("MIN Velocity (" + QString::number(MIN_VELOCITY, 'f', 1) + "Km/s)");
    lrSeries->setName("Linear Regression");

    int difftime;
    double maxSec, minSec;

    for(int i=0;i<oriStaList.size();i++)
    {
        difftime = oriStaList.at(i).maxPGATime[CHAN] - eqTime;

        if(oriStaList.at(i).distance > THRESHOLD_FOR_DIST)
        {
            //qDebug() << oriStaList.at(i).staName << oriStaList.at(i).distance;
            deadDataSeries->append(oriStaList.at(i).distance, difftime);
        }
        else
        {
            maxSec = oriStaList.at(i).distance / MAX_VELOCITY;
            minSec = oriStaList.at(i).distance / MIN_VELOCITY;

            if((double)difftime >= maxSec && (double)difftime <= minSec)
            {
                liveDataSeries->append(oriStaList.at(i).distance, difftime);
                resultStaListLevel1.push_back(oriStaList.at(i));
            }
            else
                deadDataSeries->append(oriStaList.at(i).distance, difftime);
        }
    }

    //qDebug() << deadDataSeries->count();
    ui->nError->setText(QString::number(deadDataSeries->count()));

    maxWaveSeries->append(1, 1 / MAX_VELOCITY);
    maxWaveSeries->append(THRESHOLD_FOR_DIST, THRESHOLD_FOR_DIST / MAX_VELOCITY);
    minWaveSeries->append(1, 1 / MIN_VELOCITY);
    minWaveSeries->append(THRESHOLD_FOR_DIST, THRESHOLD_FOR_DIST / MIN_VELOCITY);

    QVector<double> x;
    QVector<double> y;

    for(int i=0;i<liveDataSeries->count();i++)
    {
        x.push_back(liveDataSeries->at(i).x());
        y.push_back(liveDataSeries->at(i).y());
    }

    double m, b, r;
    linregVector(liveDataSeries->count(), x, y, &m, &b, &r);  // (m*x)+b = regression
    slope = m;
    intercept = b;
    coeff = r;

    double pgaVelocity = 1 / (((m*5)+b) - ((m*4)+b)); // if x = 5
    ui->pgaVel->setText(QString::number(pgaVelocity, 'f', 2) + "Km");

    lrSeries->append(1, (m*1)+b);
    lrSeries->append(THRESHOLD_FOR_DIST, (m*THRESHOLD_FOR_DIST)+b);

    eqChart1->addSeries(liveDataSeries);
    eqChart1->addSeries(deadDataSeries);
    eqChart1->addSeries(maxWaveSeries);
    eqChart1->addSeries(minWaveSeries);
    eqChart1->addSeries(lrSeries);

    QValueAxis *axisX = new QValueAxis();
    QValueAxis *axisY = new QValueAxis();
    axisX->setTitleText("DISTANCE (Km)");
    axisY->setTitleText("TIME (s)");
    axisX->setLabelFormat("%i");
    axisY->setLabelFormat("%i");

    axisX->setRange(0, THRESHOLD_FOR_DIST);
    axisY->setRange(0, THRESHOLD_FOR_DIST / MIN_VELOCITY);
    axisX->setTickCount(THRESHOLD_FOR_DIST/50 + 1);
    axisY->setTickCount((THRESHOLD_FOR_DIST / MIN_VELOCITY)/20 + 1);

    eqChart1->addAxis(axisX, Qt::AlignBottom);
    eqChart1->addAxis(axisY, Qt::AlignLeft);
    liveDataSeries->attachAxis(axisX);
    liveDataSeries->attachAxis(axisY);
    deadDataSeries->attachAxis(axisX);
    deadDataSeries->attachAxis(axisY);
    maxWaveSeries->attachAxis(axisX);
    maxWaveSeries->attachAxis(axisY);
    minWaveSeries->attachAxis(axisX);
    minWaveSeries->attachAxis(axisY);
    lrSeries->attachAxis(axisX);
    lrSeries->attachAxis(axisY);

    eqChartView1->setRenderHint(QPainter::Antialiasing);
    eqChartView1->setChart(eqChart1);
    ui->graph1LO->addWidget(eqChartView1);
}
