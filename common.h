#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QDateTime>
#include <QFile>
#include <QDebug>
#include <QColor>

#include <math.h>
#include <proj_api.h>

// MAP specification
#define XWIDTH          834896.18
#define YHEIGHT         656784.99
#define XORIGIN         13859276.60
#define YORIGIN         3673543.20

#define SCREEN_WIDTH 814
#define SCREEN_HEIGHT 768
#define MAXSTACNT 1000

#define CHAN 0  // 0 : horizontal PGA, 1 : 3D PGA

#define MAX_DISTANCE 10000

#define EQ_DEPTH 15
#define REGRESSION_PERCENT 0.7
#define MAX_VELOCITY 7.0
#define MIN_VELOCITY 2.0
#define THRESHOLD_FOR_DIST 400
#define THRESHOLD_FOR_SECOND 10

typedef struct _event
{
    QDateTime origintime;
    double lat;
    double lon;
    double mag;
    QString description;
    int numOfSta;
} _EVENT;

typedef struct _station
{
    int index;
    QString staName;
    double lat;
    double lon;
    double distance;
    int mapX;
    int mapY;
    int maxPGATime[2]; // horizontal, 3D vector
    float maxPGA[2];
    double predictedMAXPGA;
} _STATION;

typedef struct _point
{
    int index;
    int landX;
    int landY;
    double mapZ;
    QList<_STATION> staList;
    QList<double> mapLUT;
} _POINT;

static QColor intenColor[10] = { QColor("#FFFFFF"), QColor("#A5DDF9"), QColor("#92D050"), QColor("#FFFF00"), QColor("#FFC000"),
                                QColor("#FF0000"), QColor("#A32777"), QColor("#632523"), QColor("#4C2600"), QColor("#000000") };
//static double intenValue[9] = { 0.68, 2.24, 7.44, 25.10, 67.29, 144.49, 310.57, 667.16, 1432.172 }; // MPSS
//static double intenValue[9] = { 0.98, 2.94, 4.90, 23.52, 65.66, 127.40, 235.20, 431.20, 813.40 }; // KMA
static double intenValue[9] = { 0.68, 2.24, 3.72, 12.55, 67.29, 144.49, 310.57, 667.16, 1432.172 }; // NEW

static void getMapZValue(_POINT &point)
{
    double A=0., B=0., z=0., W=0.;
    int i;

    for(i=0;i<point.staList.size();i++)
    {
        if(point.staList.at(i).maxPGA[CHAN] == 0) continue;

        if(point.landX == point.staList.at(i).mapX && point.landY == point.staList.at(i).mapY)
        {
            A = point.staList.at(i).maxPGA[CHAN] ;
            B = 1 ;
            break ;
        }

        z = point.staList.at(i).maxPGA[CHAN] ;
        W = point.mapLUT.at(i);

        A  += ( W * z ) ;
        B  += ( W ) ;
    }
    point.mapZ = A/B;
};

inline static double sqr(double x)
{
    return x*x;
}

static int linregVector(int n, QVector<double> x, QVector<double> y, double* m, double* b, double* r)
{
    double   sumx = 0.0;                      /* sum of x     */
    double   sumx2 = 0.0;                     /* sum of x**2  */
    double   sumxy = 0.0;                     /* sum of x * y */
    double   sumy = 0.0;                      /* sum of y     */
    double   sumy2 = 0.0;                     /* sum of y**2  */

    for (int i=0;i<n;i++)
    {
        sumx  += x.at(i);
        sumx2 += sqr(x.at(i));
        sumxy += x.at(i) * y.at(i);
        sumy  += y.at(i);
        sumy2 += sqr(y.at(i));
    }

    double denom = (n * sumx2 - sqr(sumx));
    if (denom == 0)
    {
        // singular matrix. can't solve the problem.
        *m = 0;
        *b = 0;
        if (r) *r = 0;
            return 1;
    }

    *m = (n * sumxy  -  sumx * sumy) / denom;
    *b = (sumy * sumx2  -  sumx * sumxy) / denom;
    if (r!=NULL)
    {
        *r = (sumxy - sumx * sumy / n) /    /* compute correlation coeff */
              sqrt((sumx2 - sqr(sumx)/n) *
              (sumy2 - sqr(sumy)/n));
    }

    return 0;
}

static double getPredictedValue(double dist, double mag)
{
    double y = (dist * dist) + (EQ_DEPTH * EQ_DEPTH);
    double R = sqrt(y);
    double c0, c1, c2;

    //qDebug() << dist << R;

    /* 2001  */
    double ksai0[4] = {0.1250737E+02, 0.4874629E+00, -0.2940726E-01, 0.1737204E-01};
    double ksai1[4] = {-0.1928185E-02, 0.2251016E-03, -0.6378615E-04, 0.6967121E-04};
    double ksai2[4] = {-0.5795112E+00, 0.1138817E+00, -0.1162326E-01, -0.3646674E-02};

    c0 = ksai0[0] + ksai0[1]*(mag-6) + ksai0[2]*pow((mag-6), 2) + ksai0[3]*pow((mag-6), 3);
    c1 = ksai1[0] + ksai1[1]*(mag-6) + ksai1[2]*pow((mag-6), 2) + ksai1[3]*pow((mag-6), 3);
    c2 = ksai2[0] + ksai2[1]*(mag-6) + ksai2[2]*pow((mag-6), 2) + ksai2[3]*pow((mag-6), 3);


    /* 2003
    double ksai0[4] = {0.1073829E+02 , 0.5909022E+00, -0.5622945E-01, 0.2135007E-01};
    double ksai1[4] = {-0.2379955E-02, 0.2081359E-03, -0.2046806E-04, 0.4192630E-04};
    double ksai2[4] = {-0.2437218E+00, 0.9498274E-01, -0.8804236E-02, -0.3302350E-02};

    c0 = ksai0[0] + (ksai0[1]*(mag-6)) + pow((ksai0[2]*(mag-6)), 2) + pow((ksai0[3]*(mag-6)), 3);
    c1 = ksai1[0] + (ksai1[1]*(mag-6)) + pow((ksai1[2]*(mag-6)), 2) + pow((ksai1[3]*(mag-6)), 3);
    c2 = ksai2[0] + (ksai2[1]*(mag-6)) + pow((ksai2[2]*(mag-6)), 2) + pow((ksai2[3]*(mag-6)), 3);
     */

    double lnSA;
    if(R < 100)
        lnSA = c0 + c1*R + c2*log(R) - log(R) - 0.5*log(100);
    else if(R > 100)
        lnSA = c0 + c1*R + c2*log(R) - log(100) - 0.5*log(R);
    else if(R == 100)
        lnSA = c0 + c1*R + c2*log(R) - log(100) - 0.5*log(100);

    //qDebug() << c0 << c1 << c2 << R << exp(lnSA) << qLn(lnSA);

    return exp(lnSA);
}

static int redColor(float gal)
{
  int color ;

  // red color value
  if( gal <= 0.0098 )
  {
    color = 191 ;
  }
  else if (gal > 0.0098 && gal <= 0.0392)
  {
    color = gal * (-3265.31) + 223 ;
  }
  else if (gal > 0.0392 && gal <= 0.0784)
  {
    color = 95 ;
  }
  else if (gal > 0.0784 && gal <= 0.098)
  {
    color = gal * 3265.31 - 161 ;
  }
  else if (gal > 0.098 && gal <= 0.98)
  {
    color = gal * 103.82 + 148.497 ;
  }
  else if (gal > 0.98 && gal <= 147)
  {
    color = 255 ;
  }
  else if (gal > 147 && gal <= 245)
  {
    color = -0.00333195 * pow(gal,2) + 0.816327 * gal + 207 ;
  }
  else if (gal > 245)
    color = 207 ;

  return color ;
}

static int greenColor(float gal)
{
  int color ;
  // red color value
  if( gal <= 0.98 )
  {
    color = 255 ;
  }
  else if (gal > 0.98 && gal <= 9.8)
  {
    color = -0.75726 * gal * gal - 0.627943 * gal + 255.448 ;
  }
  else if (gal > 0.98 && gal <= 245)
  {
    color = 0.00432696 * gal * gal - 1.84309 * gal + 192.784 ;
    if(color < 0)
      color = 0 ;
  }
  else if (gal > 245)
    color = 0 ;

  return color ;
}

static int blueColor(float gal)
{
  int color ;

  // red color value
  if( gal <= 0.0098 )
  {
    color = 255 ;
  }
  else if (gal > 0.0098 && gal <= 0.098)
  {
    color = -19799.2 * gal * gal + 538.854 * gal + 260.429 ;
  }
  else if (gal > 0.098 && gal <= 0.98)
  {
    color = -35.4966 * gal * gal - 65.8163 * gal + 116.264 ;
  }
  else if (gal > 0.98 && gal <= 3.92)
  {
    color = -5.10204 * gal + 20 ;
  }
  else if (gal > 3.92)
  {
    color = 0 ;
  }

  if(color > 255)
    color = 255 ;

  return color ;
}

#endif // COMMON_H
