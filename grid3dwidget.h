#ifndef GRID3DWIDGET_H
#define GRID3DWIDGET_H

#include <QWidget>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <QMatrix4x4>
#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>
#include <QToolTip>

class Grid3DWidget : public QWidget
{
    Q_OBJECT

public:
    Grid3DWidget(QWidget *parent = 0);
    ~Grid3DWidget();

    QMatrix4x4 matrix;
    double  scaleFactor;        //米->像素
    QPointF ptMiddle;           //屏幕中心点，视线中心线上的点

    float   maxDis;             //米
    float   minDis;
    float   gridRatio;          //场景宽高比

    //
    float   rotateX;
    float   rotateY;
    float   rotateZ;
    float   h;
    float   step;

    float   verticalAngle;
    float   aspectRatio;
    float   nearPlane;
    float   farPlane;

    float   A;
    float   B;
    float   C;
    float   D;

    QPointF scenePt;

    bool    prepareData();
    QPoint  ptScene2Screen(QPointF);
    QPointF ptScreen2Scene(QPoint);

    virtual void    showEvent(QShowEvent*);
    virtual void    mouseMoveEvent(QMouseEvent*);
    virtual void    paintEvent(QPaintEvent*);
    void    paintGrid(QPainter*);


    void    originPaintGrid(QPainter*);  //仅留作纪念
};

#endif // GRID3DWIDGET_H
