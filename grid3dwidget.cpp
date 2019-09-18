#include "grid3dwidget.h"
#include <math.h>

Grid3DWidget::Grid3DWidget(QWidget *parent)
    : QWidget(parent)
{
    h = 200;
    rotateX = -30;
    rotateY = 0;
    rotateZ = 0;

    verticalAngle = 90;
    aspectRatio = 16/9.0;
    nearPlane = 0.1;
    farPlane = 1000;

    update();
}

Grid3DWidget::~Grid3DWidget()
{

}



void Grid3DWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawLine(QPoint(0, this->height()/2), QPoint(this->width(), this->height()/2));
    painter.drawLine(QPoint(this->width()/2, 0), QPoint(this->width()/2, this->height()));

    paintGrid(&painter);
}

void Grid3DWidget::paintGrid(QPainter* painter)
{
    float step = 40;
    float maxDis = h/tan(-rotateX*M_PI/180);
    painter->drawText(QPoint(0, this->height()/2), QString::number(maxDis));
    float minDis = 0;

    float maxZVal = maxDis/cos(rotateX*M_PI/180);
    float minZVal = maxZVal-(maxDis-minDis)*cos(rotateX*M_PI/180);

    QMatrix4x4 mat;

    mat.translate(this->width()/2, this->height(), 0);
    mat.rotate(rotateX, 1, 0, 0);
    mat.rotate(rotateY, 0, 1, 0);
    mat.rotate(rotateZ, 0, 0, 1);
    mat.translate(0,0, -h);
    mat.perspective(verticalAngle,aspectRatio,nearPlane,farPlane);


    QVector3D pFar(0, maxDis, maxZVal);
    QPointF pF = mat.map(pFar).toPointF();
    float near2FarDis = maxDis * sin(verticalAngle*M_PI/180/2) / sin((-rotateX+verticalAngle/2)*M_PI/180);//地面上中心点与最近点米数
    painter->drawText(QPoint(0, this->height()), QString::number(maxDis-near2FarDis));

    QVector3D pNear(0, maxDis-near2FarDis, maxZVal-near2FarDis*cos(rotateX*M_PI/180));
    QPointF pN = mat.map(pNear).toPointF();
    double factor = this->height()/2 / fabs(pF.y()-pN.y());

    painter->translate(this->width()/2-pF.x(), this->height()/2-pF.y());
    for(int i=maxDis; i>=minDis; i-=step)
    {
        float zVal = maxZVal-(maxDis-i)*cos(rotateX*M_PI/180);

        QVector3D pH1(-(maxDis-minDis)/2, i, zVal);
        QPointF ph1 = mat.map(pH1).toPointF();
        QVector3D pH2((maxDis-minDis)/2, i, zVal);
        QPointF ph2 = mat.map(pH2).toPointF();

        // H line
        painter->drawLine(QPoint(pF.x()+(ph1.x()-pF.x())*factor, pF.y()+(ph1.y()-pF.y())*factor),
                          QPoint(pF.x()+(ph2.x()-pF.x())*factor, pF.y()+(ph2.y()-pF.y())*factor));
    }
    for(int i=minDis; i<=maxDis; i+=step)
    {
        //右竖线
        QVector3D pV1((i-minDis)/2, maxDis, maxZVal);
        QPointF pv1 = mat.map(pV1).toPointF();
        QVector3D pV2((i-minDis)/2, minDis, minZVal);
        QPointF pv2 = mat.map(pV2).toPointF();

        //左竖线
        QVector3D pV3(-(i-minDis)/2, maxDis, maxZVal);
        QPointF pv3 = mat.map(pV3).toPointF();
        QVector3D pV4(-(i-minDis)/2, minDis, minZVal);
        QPointF pv4 = mat.map(pV4).toPointF();

        // V line
        painter->drawLine(QPoint(pF.x()+(pv1.x()-pF.x())*factor, pF.y()+(pv1.y()-pF.y())*factor),
                          QPoint(pF.x()+(pv2.x()-pF.x())*factor, pF.y()+(pv2.y()-pF.y())*factor));
        painter->drawLine(QPoint(pF.x()+(pv3.x()-pF.x())*factor, pF.y()+(pv3.y()-pF.y())*factor),
                          QPoint(pF.x()+(pv4.x()-pF.x())*factor, pF.y()+(pv4.y()-pF.y())*factor));
    }

}
