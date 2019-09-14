#ifndef GRID3DWIDGET_H
#define GRID3DWIDGET_H

#include <QWidget>
#include <Qt3DCore/qentity.h>
#include <Qt3DCore/qtransform.h>
#include <QMatrix4x4>
#include <QPaintEvent>
#include <QPainter>

class Grid3DWidget : public QWidget
{
    Q_OBJECT

public:
    Grid3DWidget(QWidget *parent = 0);
    ~Grid3DWidget();

    float   rotateX;
    float   rotateY;
    float   rotateZ;
    float   h;

    float   verticalAngle;
    float   aspectRatio;
    float   nearPlane;
    float   farPlane;

    void    paintEvent(QPaintEvent*);
    void    paintGrid(QPainter*);
};

#endif // GRID3DWIDGET_H
