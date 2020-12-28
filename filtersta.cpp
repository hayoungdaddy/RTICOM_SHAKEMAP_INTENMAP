#include "filtersta.h"
#include "ui_filtersta.h"

FilterSta::FilterSta(QList<_STATION> staList, double eewMag, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FilterSta)
{
    ui->setupUi(this);

    oriStaVT = staList;

    pgaVsDistView = new QChartView();
    pgaVsDistChart = new QChart();

    //get pred use jo and bagg
    for(int i=0;i<1000;i++)
    {
        predVector.push_back(getPredictedValue(i+1, eewMag));
        predVectorUp.push_back(predVector.at(i) + (predVector.at(i) * REGRESSION_PERCENT));
        predVectorDown.push_back(predVector.at(i) - (predVector.at(i) * REGRESSION_PERCENT));
    }

    makePGAvsDistGraph();
}

FilterSta::~FilterSta()
{
    delete ui;
}

void FilterSta::makePGAvsDistGraph()
{
    /* jo and baag (2001) */
    int nUsed = 0;
    double maxValue = 0;

    QScatterSeries *realDataSeries = new QScatterSeries();
    realDataSeries->setName("Final Stations");
    realDataSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    realDataSeries->setMarkerSize(10.0);

    QScatterSeries *outDataSeries = new QScatterSeries();
    outDataSeries->setName("OutOfBounds");
    outDataSeries->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    outDataSeries->setMarkerSize(10.0);

    QLineSeries *predDataSeries = new QLineSeries();
    predDataSeries->setName("Pred.");
    QLineSeries *predDataSeriesUp = new QLineSeries();
    predDataSeriesUp->setName("Pred.(+" + QString::number(REGRESSION_PERCENT*100) + "%)");
    QLineSeries *predDataSeriesDown = new QLineSeries();
    predDataSeriesDown->setName("Pred.(-" + QString::number(REGRESSION_PERCENT*100) + "%)");

    for(int i=0;i<oriStaVT.count();i++)
    {
        double predUpPGA = predVectorUp.at((int)oriStaVT.at(i).distance-1);
        double predDownPGA = predVectorDown.at((int)oriStaVT.at(i).distance-1);

        if(oriStaVT.at(i).maxPGA[CHAN] <= predUpPGA && oriStaVT.at(i).maxPGA[CHAN] >= predDownPGA)
        {
            nUsed++;
            realDataSeries->append(oriStaVT.at(i).distance, oriStaVT.at(i).maxPGA[CHAN]);
            if(maxValue < oriStaVT.at(i).maxPGA[CHAN])
                maxValue = oriStaVT.at(i).maxPGA[CHAN];

            _STATION sta;
            sta = oriStaVT.at(i);
            sta.predictedMAXPGA = predVector.at(int(sta.distance)-1);
            filteredStaVT.push_back(sta);

            sta.maxPGA[CHAN] = sta.predictedMAXPGA;
            predStaList.push_back(sta);
        }
        else
        {
            //qDebug() << oriStaVT.at(i).maxPGA[CHAN] << predUpPGA << predDownPGA;
            outDataSeries->append(oriStaVT.at(i).distance, oriStaVT.at(i).maxPGA[CHAN]);
        }
    }

    for(int i=0;i<1000;i++)
    {
        predDataSeries->append(i+1, predVector.at(i));
        predDataSeriesUp->append(i+1, predVectorUp.at(i));
        predDataSeriesDown->append(i+1, predVectorDown.at(i));
    }

    pgaVsDistChart->addSeries(realDataSeries);
    pgaVsDistChart->addSeries(outDataSeries);
    pgaVsDistChart->addSeries(predDataSeries);
    pgaVsDistChart->addSeries(predDataSeriesUp);
    pgaVsDistChart->addSeries(predDataSeriesDown);

    QLogValueAxis *axisLogX = new QLogValueAxis();
    axisLogX->setTitleText("Distance (Km)");
    axisLogX->setLabelFormat("%i");
    axisLogX->setMinorTickCount(-1);

    pgaVsDistChart->addAxis(axisLogX, Qt::AlignBottom);
    realDataSeries->attachAxis(axisLogX);
    outDataSeries->attachAxis(axisLogX);
    predDataSeries->attachAxis(axisLogX);
    predDataSeriesUp->attachAxis(axisLogX);
    predDataSeriesDown->attachAxis(axisLogX);

    QLogValueAxis *axisLogY = new QLogValueAxis();
    axisLogY->setTitleText("PGA (gal)");
    axisLogY->setLabelFormat("%f");
    axisLogY->setMinorTickCount(-1);

    pgaVsDistChart->addAxis(axisLogY, Qt::AlignLeft);
    realDataSeries->attachAxis(axisLogY);
    outDataSeries->attachAxis(axisLogY);
    predDataSeries->attachAxis(axisLogY);
    predDataSeriesUp->attachAxis(axisLogY);
    predDataSeriesDown->attachAxis(axisLogY);

    ui->nTotalLB->setText(QString::number(oriStaVT.count()));
    ui->nErrorLB->setText(QString::number(oriStaVT.count()-nUsed));
    ui->nUsedLB->setText(QString::number(nUsed));

    // area chart for intensity
    QLineSeries *intenSeries[9];

    for(int i=0;i<9;i++)
    {
        if(maxValue < intenValue[i])
            break;

        intenSeries[i] = new QLineSeries();
        intenSeries[i]->append(1, intenValue[i]);
        intenSeries[i]->append(1000, intenValue[i]);
        QString name; if(i==0) name = "I"; else if(i==1) name = "II"; else if(i==2) name = "III"; else if(i==3) name = "IV"; else if(i==4) name = "V";
        else if(i==5) name = "VI"; else if(i==6) name = "VII"; else if(i==7) name = "VIII"; else if(i==8) name = "IX";
        intenSeries[i]->setName(name + "(" + QString::number(intenValue[i], 'f', 2) + "gal)");
        QPen pen; pen.setWidth(2);
        if(i ==0)
            pen.setBrush(QBrush(QColor("#000000")));
        else
            pen.setBrush(QBrush(QColor(intenColor[i])));
        intenSeries[i]->setPen(pen);
        pgaVsDistChart->addSeries(intenSeries[i]);
        intenSeries[i]->attachAxis(axisLogX);
        intenSeries[i]->attachAxis(axisLogY);
    }

    pgaVsDistView->setRenderHint(QPainter::Antialiasing);
    pgaVsDistView->setChart(pgaVsDistChart);
    ui->gpLO->addWidget(pgaVsDistView);
}
