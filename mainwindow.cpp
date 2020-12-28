#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "skorea_landXY.h"

#include <QPainter>
#include <QPaintEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->filter1CB, SIGNAL(stateChanged(int)), this, SLOT(filter1Changed(int)));
    connect(ui->showFilter1PB, SIGNAL(clicked()), this, SLOT(showFilter1()));
    connect(ui->filter2CB, SIGNAL(stateChanged(int)), this, SLOT(filter2Changed(int)));
    connect(ui->showFilter2PB, SIGNAL(clicked()), this, SLOT(showFilter2()));
    connect(ui->predCB, SIGNAL(stateChanged(int)), this, SLOT(showPredict(int)));

    initProj();

    isFilterLevel1 = false;
    isFilterLevel2 = false;

    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/temp.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/CY3.4_PGA_DIS_latlon_KMA_V2.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/ChangNyung_M3_4_allStations.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/SangJu_QSCD20_AllSta.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/KG_KS/ChangNyung_M3_4_KG_KS.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/KG_KS/SangJu_QSCD20_KG_KS.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/20190419_021637_M4_62.dat");

    //ALL
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/ALL/20190225093034_YUNGYANG_M3_3_ALL_QSCD.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/ALL/20190419111637_DONGHE_M4_6_ALL_QSCD.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/ALL/20190721110418_SANGJU_M4_1_ALL_QSCD.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/ALL/20191027153727_CHANGNYUNG_M3_3_ALL_QSCD.dat");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/ALL/20191230003209_MILYANG_M3_5_ALL_QSCD.dat");

    //KG_KS
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/KG_KS/20190225093034_YUNGYANG_M3_3_HG.txt");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/KG_KS/20190419111637_DONGHE_M4_6_HG.txt");
    readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/KG_KS/20190721110418_SANGJU_M4_1_HG.txt");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/KG_KS/20191027153727_CHANGNYUNG_M3_3_HG.txt");
    //readEventFile("/home/sysop/RTICOM2_EVENTS_DATA_FOR_SHAKEMAP/KG_KS/20191230003209_MILYANG_M3_5_HG.txt");

    ui->originTimeLB->setText(event.origintime.toString("yyyy/MM/dd hh:mm:ss"));
    ui->latLB->setText(QString::number(event.lat, 'f', 4));
    ui->lonLB->setText(QString::number(event.lon, 'f', 4));
    ui->magLB->setText(QString::number(event.mag, 'f', 1));

    oriPoints = init_MSM(oriStaList, 0);
    oriPoints = processShakeMap(oriStaList, oriPoints);

    timedistcurvefilter = new TimeDistCurveFilter(oriStaList, event.origintime.toTime_t());
    timedistCurveFilterStaList = timedistcurvefilter->resultStaList;
    timedistCurveFilterPoints = init_MSM(timedistCurveFilterStaList, 1);
    timedistCurveFilterPoints = processShakeMap(timedistCurveFilterStaList, timedistCurveFilterPoints);

    gmpefiltersta = new FilterSta(oriStaList, event.mag);
    gmpeFilteredStaList = gmpefiltersta->filteredStaVT;
    gmpePoints = init_MSM(gmpeFilteredStaList, 2);
    gmpePoints = processShakeMap(gmpeFilteredStaList, gmpePoints);

    allfiltersta = new FilterSta(timedistCurveFilterStaList, event.mag);
    allFilteredStaList = allfiltersta->filteredStaVT;
    allFilteredPoints = init_MSM(allFilteredStaList, 3);
    allFilteredPoints = processShakeMap(allFilteredStaList, allFilteredPoints);

    predStaList = gmpefiltersta->predStaList;
    predPoints = init_MSM(predStaList, 4);
    predPoints = processShakeMap(predStaList, predPoints);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QList<_POINT> MainWindow::processShakeMap(QList<_STATION> slist, QList<_POINT> points)
{
    for(int i=0;i<points.size();i++)
    {
        _POINT mypoint = points.at(i);
        mypoint.staList = slist;
        points.replace(i, mypoint);
    }

    QFuture<void> future = QtConcurrent::map(points, getMapZValue);
    future.waitForFinished();

    return points;
}

void MainWindow::showPredict(int state)
{
    if(state == 0)
    {
        isPredict = false;
        repaint();
    }
    else
    {
        isPredict = true;
        repaint();
    }
}

void MainWindow::filter1Changed(int state)
{
    if(state == 0)
    {
        isFilterLevel1 = false;
        repaint();
    }
    else
    {
        isFilterLevel1 = true;
        repaint();
    }
}

void MainWindow::showFilter1()
{
    if(timedistcurvefilter->isHidden())
        timedistcurvefilter->show();
}

void MainWindow::filter2Changed(int state)
{
    if(state == 0)
    {
        isFilterLevel2 = false;
        repaint();
    }
    else
    {
        isFilterLevel2 = true;
        repaint();
    }
}

void MainWindow::showFilter2()
{
    if(isFilterLevel1 == true && isFilterLevel2 == true)
    {
        if(allfiltersta->isHidden())
            allfiltersta->show();
    }
    else if(isFilterLevel1 == false && isFilterLevel2 == true)
    {
        if(gmpefiltersta->isHidden())
            gmpefiltersta->show();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter1(ui->openGLWG1);
    painter1.setRenderHint(QPainter::Antialiasing);
    paint(&painter1, event, 0);
    painter1.end();

    QPainter painter2(ui->openGLWG2);
    painter2.setRenderHint(QPainter::Antialiasing);
    paint(&painter2, event, 1);
    painter2.end();

    QPainter painter3(ui->openGLWG3);
    painter3.setRenderHint(QPainter::Antialiasing);
    paint(&painter3, event, 2);
    painter3.end();
}

void MainWindow::paint(QPainter *painter, QPaintEvent *event, int type)
{
    QList<_POINT> disPoints;
    QList<_STATION> disStaList;
    int mapX, mapY;
    if(isPredict)
    {
        disPoints = predPoints;
        disStaList = predStaList;
        mapX = predMaxX;
        mapY = predMaxY;
    }
    else if(isFilterLevel1 == false && isFilterLevel2 == false)
    {
        disPoints = oriPoints;
        disStaList = oriStaList;
        mapX = oriMaxX;
        mapY = oriMaxY;
    }
    else if(isFilterLevel1 == true && isFilterLevel2 == false)
    {
        disPoints = timedistCurveFilterPoints;
        disStaList = timedistCurveFilterStaList;
        mapX = timeDistCurveMaxX;
        mapY = timeDistCurveMaxY;
    }
    else if(isFilterLevel1 == false && isFilterLevel2 == true)
    {
        disPoints = gmpePoints;
        disStaList = gmpeFilteredStaList;
        mapX = gmpeMaxX;
        mapY = gmpeMaxY;
    }
    else if(isFilterLevel1 == true && isFilterLevel2 == true)
    {
        disPoints = allFilteredPoints;
        disStaList = allFilteredStaList;
        mapX = allMaxX;
        mapY = allMaxY;
    }

    ui->nStaLB->setText(QString::number(disStaList.size()));

    QRect rectI(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    painter->fillRect(rectI, QBrush(QColor(Qt::white)));
    QImage image("/home/sysop/GIT/RTICOM2_PLAYER/RTICOM2_PLAYER_V2_6/skorea.png");
    painter->drawImage(rectI, image);

    int i;

    if(type == 0)
    {
        painter->setPen(QPen(Qt::black));
        _STATION sta;

        for(i=0;i<disStaList.size();i++)
        {
            sta = disStaList.at(i);
            painter->setBrush(QBrush(QColor(redColor(sta.maxPGA[CHAN]), greenColor(sta.maxPGA[CHAN]), blueColor(sta.maxPGA[CHAN]))));
            painter->drawEllipse(QPoint(sta.mapX, sta.mapY), 5, 5);
        }

        painter->setPen(QPen(Qt::red));
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->drawEllipse(QPoint(mapX, mapY), 10, 10);

    }
    else if(type==1)
    {
        painter->setPen(QPen(Qt::NoPen));
        int i;
        _POINT mypoint;

        for(i=0;i<LANDXYCNT;i++)
        {
            mypoint = disPoints.at(i);

            if(mypoint.mapZ >= 9.9 && mypoint.mapZ <= 10.1 )
                painter->setBrush(QColor(255, 0, 0));
            else
                painter->setBrush(QColor(redColor(mypoint.mapZ), greenColor(mypoint.mapZ), blueColor(mypoint.mapZ)));

            painter->drawRect(QRectF(mypoint.landX, mypoint.landY, 1, 1));
        }
    }
    else if(type == 2)
    {
        painter->setPen(QPen(Qt::NoPen));
        _POINT mypoint;

        for(i = 0 ; i < LANDXYCNT ; i ++)
        {
            mypoint = disPoints.at(i);

            if(mypoint.mapZ <= intenValue[0])
                continue;
            else if(mypoint.mapZ > intenValue[0] && mypoint.mapZ <= intenValue[1])
                painter->setBrush(intenColor[1]);
            else if(mypoint.mapZ > intenValue[1] && mypoint.mapZ <= intenValue[2])
                painter->setBrush(intenColor[2]);
            else if(mypoint.mapZ > intenValue[2] && mypoint.mapZ <= intenValue[3])
                painter->setBrush(intenColor[3]);
            else if(mypoint.mapZ > intenValue[3] && mypoint.mapZ <= intenValue[4])
                painter->setBrush(intenColor[4]);
            else if(mypoint.mapZ > intenValue[4] && mypoint.mapZ <= intenValue[5])
                painter->setBrush(intenColor[5]);
            else if(mypoint.mapZ > intenValue[5] && mypoint.mapZ <= intenValue[6])
                painter->setBrush(intenColor[6]);
            else if(mypoint.mapZ > intenValue[6] && mypoint.mapZ <= intenValue[7])
                painter->setBrush(intenColor[7]);
            else if(mypoint.mapZ > intenValue[7] && mypoint.mapZ <= intenValue[8])
                painter->setBrush(intenColor[8]);
            else if(mypoint.mapZ > intenValue[8])
                painter->setBrush(intenColor[9]);
            else
                painter->setBrush(QColor(Qt::gray));

            painter->drawRect(QRect(mypoint.landX, mypoint.landY, 1, 1));
        }

        // draw legend
        QFont font("Times", 11, QFont::Bold);
        painter->setFont(font);
        for(i=0;i<10;i++)
        {
            painter->setPen(QPen(Qt::black));
            painter->setBrush(QBrush(intenColor[i]));
            QRect recL(310 + (i*50), 709, 50, 51);
            painter->drawRect(recL);

            if(i==1 || i==2 || i==5 || i==6 || i==7 || i==8 || i==9) painter->setPen(QPen(Qt::white));

            QString str;
            if(i==0) str = "I";
            else if(i==1) str = "II";
            else if(i==2) str = "III";
            else if(i==3) str = "IV";
            else if(i==4) str = "V";
            else if(i==5) str = "VI";
            else if(i==6) str = "VII";
            else if(i==7) str = "VIII";
            else if(i==8) str = "IX";
            else if(i==9) str = "X+";
            painter->drawText(recL, Qt::AlignCenter, str);
        }

        // tempolary drawing station on intensity map
        /*
        painter->setPen(QPen(Qt::black));
        _STATION sta;

        QFont font2("Times", 7, QFont::Normal);
        painter->setFont(font2);
        for(i=0;i<disStaList.size();i++)
        {
            sta = disStaList.at(i);

            if(sta.maxPGA[CHAN] <= intenValue[0])
                painter->setBrush(QBrush(Qt::NoBrush));
            else if(sta.maxPGA[CHAN] > intenValue[0] && sta.maxPGA[CHAN] <= intenValue[1])
                painter->setBrush(intenColor[1]);
            else if(sta.maxPGA[CHAN] > intenValue[1] && sta.maxPGA[CHAN] <= intenValue[2])
                painter->setBrush(intenColor[2]);
            else if(sta.maxPGA[CHAN] > intenValue[2] && sta.maxPGA[CHAN] <= intenValue[3])
                painter->setBrush(intenColor[3]);
            else if(sta.maxPGA[CHAN] > intenValue[3] && sta.maxPGA[CHAN] <= intenValue[4])
                painter->setBrush(intenColor[4]);
            else if(sta.maxPGA[CHAN] > intenValue[4] && sta.maxPGA[CHAN] <= intenValue[5])
                painter->setBrush(intenColor[5]);
            else if(sta.maxPGA[CHAN] > intenValue[5] && sta.maxPGA[CHAN] <= intenValue[6])
                painter->setBrush(intenColor[6]);
            else if(sta.maxPGA[CHAN] > intenValue[6] && sta.maxPGA[CHAN] <= intenValue[7])
                painter->setBrush(intenColor[7]);
            else if(sta.maxPGA[CHAN] > intenValue[7] && sta.maxPGA[CHAN] <= intenValue[8])
                painter->setBrush(intenColor[8]);
            else if(sta.maxPGA[CHAN] > intenValue[8])
                painter->setBrush(intenColor[9]);
            else
                painter->setBrush(QColor(Qt::gray));

            //painter->setBrush(QBrush(QColor(redColor(sta.maxPGA[CHAN]), greenColor(sta.maxPGA[CHAN]), blueColor(sta.maxPGA[CHAN]))));
            painter->drawEllipse(QPoint(sta.mapX, sta.mapY), 5, 5);
            painter->drawText(QRect(sta.mapX - 15 , sta.mapY + 5, 30, 15), Qt::AlignCenter, sta.staName);
        }


        painter->setPen(QPen(Qt::red));
        painter->setBrush(QBrush(Qt::NoBrush));
        painter->drawEllipse(QPoint(mapX, mapY), 10, 10);
        */
    }

    painter->setPen(QPen(Qt::red));
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawEllipse(QPoint(originX, originY), (int)(50 * 1.2), (int)(50 * 1.2));
    painter->setPen(QPen(Qt::blue));
    painter->drawEllipse(QPoint(originX, originY), (int)(100 * 1.2), (int)(100 * 1.2));
    painter->setPen(QPen(Qt::black));
    painter->drawEllipse(QPoint(originX, originY), (int)(200 * 1.2), (int)(200 * 1.2));
}

void MainWindow::readEventFile(QString fileName)
{
    QFile eventFile(fileName);
    if(!eventFile.exists())
    {
        qDebug() << "The Event File doesn't exists.";
        return;
    }

    int noData=0;

    if(eventFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&eventFile);
        QString line, _line;

        while(!stream.atEnd())
        {
            line = stream.readLine();
            _line = line.simplified();

            if(_line.startsWith(" ") || _line.startsWith("#") || _line.startsWith("STA") || _line.startsWith("origintime(human)"))
                continue;
            else if(_line.startsWith("origintime(human)"))
                continue;
            else if(_line.startsWith("STA"))
                continue;
            else if(_line.startsWith("origintime(epoch)"))
                event.origintime.setTime_t(_line.section("=", 1, 1).toInt());
            else if(_line.startsWith("latitude"))
                event.lat = _line.section("=", 1, 1).toDouble();
            else if(_line.startsWith("longitude"))
                event.lon = _line.section("=", 1, 1).toDouble();
            else if(_line.startsWith("magnitude"))
                event.mag = _line.section("=", 1, 1).toDouble();
            else if(_line.startsWith("description"))
                event.description = _line.section("=", 1, 1);
            else if(_line.startsWith("numberofstations"))
            {
                event.numOfSta = _line.section("=", 1, 1).toInt();
                line = stream.readLine();
            }
            else
            {
                _STATION _sta;
                _sta.staName = _line.section(" ", 0, 0);
                /*
                if(_sta.staName.startsWith("KS") || _sta.staName.startsWith("KG"))
                {
                    continue;
                }
                */
                _sta.distance = _line.section(" ", 1, 1).toDouble();
                //if(_sta.distance > 100) continue;
                _sta.lat = _line.section(" ", 2, 2).toDouble();
                _sta.lon = _line.section(" ", 3 ,3).toDouble();
                ll2xy(pj_longlat, pj_eqc, _sta.lon, _sta.lat, &_sta.mapX, &_sta.mapY);
                _sta.maxPGATime[0] = _line.section(" ", 4, 4).toInt();
                _sta.maxPGA[0] = _line.section(" ", 5, 5).toDouble();
                if(_sta.maxPGA[0] == 0)
                {
                    noData++;
                    qDebug() << _sta.staName;
                    continue;
                }
                oriStaList.push_back(_sta);

            }
        }
        ll2xy(pj_longlat, pj_eqc, event.lon, event.lat, &originX, &originY);
        eventFile.close();
    }

    qDebug() << "Number of stations : " << oriStaList.size() << " No Data or 0 Value : " << noData;

}

void MainWindow::initProj()
{
    if (!(pj_longlat = pj_init_plus("+proj=longlat +ellps=WGS84")) )
    {
        qDebug() << "Can't initialize projection.";
        exit(1);
    }

    if (!(pj_eqc = pj_init_plus("+proj=eqc +ellps=WGS84")) )
    {
        qDebug() << "Can't initialize projection.";
        exit(1);
    }
}

QList<_POINT> MainWindow::init_MSM(QList<_STATION> staList, int type)
{
    // IDW weight
    int i, j;
    QList<_POINT> tempPoints;

    double maxdist = 0 ;
    double Rw = 0, Rp = 0 ;

    int Np =  9 ;
    int Nw = 18 ;

    maxdist = MAX_DISTANCE;

    double myMaxLat, myMaxLon;
    float myMaxPGA = 0;

    for(i=0;i<staList.size();i++)
    {
        if(staList.at(i).maxPGA[CHAN] > myMaxPGA)
        {
            myMaxPGA = staList.at(i).maxPGA[CHAN];
            myMaxLat = staList.at(i).lat;
            myMaxLon = staList.at(i).lon;
        }
    }

    if(type == 0)
        ll2xy(pj_longlat, pj_eqc, myMaxLon, myMaxLat, &oriMaxX, &oriMaxY);
    else if(type == 1)
        ll2xy(pj_longlat, pj_eqc, myMaxLon, myMaxLat, &timeDistCurveMaxX, &timeDistCurveMaxY);
    else if(type == 2)
        ll2xy(pj_longlat, pj_eqc, myMaxLon, myMaxLat, &gmpeMaxX, &gmpeMaxY);
    else if(type == 3)
        ll2xy(pj_longlat, pj_eqc, myMaxLon, myMaxLat, &allMaxX, &allMaxY);
    else if(type == 4)
        ll2xy(pj_longlat, pj_eqc, myMaxLon, myMaxLat, &predMaxX, &predMaxY);

    // now calc Rw, Rp
    Rw = maxdist / 2 * sqrt((double)Nw / (double)staList.size());
    Rp = maxdist / 2 * sqrt((double)Np / (double)staList.size());

    qDebug() << Rw << Rp << maxdist << staList.size();

    QString temp;
    QProgressDialog progress(temp, "Cancle", 0, LANDXYCNT, this);
    progress.setWindowTitle("Initial MSM");
    progress.setMinimumWidth(700);
    progress.setWindowModality(Qt::WindowModal);

    double distan, dis, W;

    for(i=0;i<LANDXYCNT;i++)
    {
        _POINT mypoint;
        mypoint.index = i;
        mypoint.landX = landXY[i][0];
        mypoint.landY = landXY[i][1];

        progress.setValue(i);

        for(j=0;j<staList.size();j++)
        {
            _STATION mysta = staList.at(j);
            distan = sqrt(pow((double)(mypoint.landX - mysta.mapX), 2) + pow((double)(mypoint.landY - mysta.mapY), 2) );
            dis = Rp - distan ;
            W = 0 ;

            if(dis >= 0)
                W  = pow( (dis / (Rp * distan)), 2) ;
            else if(dis < 0)
                W = pow( (1 / (Rp * distan)), 2) ;

            mypoint.mapLUT.push_back(W);
        }
        tempPoints.push_back(mypoint);

    }
    progress.setValue(LANDXYCNT);

    return tempPoints;
}

int MainWindow::ll2xy(projPJ src, projPJ target, float lon, float lat, int *rx, int *ry)
{
    double x, y;
    int rtn = 0 ;

    double mapx, mapy ;

    x = lon * DEG_TO_RAD ;
    y = lat * DEG_TO_RAD ;

    rtn = pj_transform(src, target, 1, 1, &x, &y, nullptr );

    mapx = x - XORIGIN ;
    mapx *=  SCREEN_WIDTH ;
    mapx /= XWIDTH ;

    mapy = y - YORIGIN ;
    mapy *=  SCREEN_HEIGHT ;
    mapy /= YHEIGHT ;
    mapy = SCREEN_HEIGHT - mapy ;

    *rx = (int)mapx ;
    *ry = (int)mapy ;
}
