#include "grid3dwidget.h"

Grid3DWidget::Grid3DWidget(QWidget *parent)
    : QWidget(parent)
{
    h = 144;
    rotateX = 0;
    rotateY = 0;
    rotateZ = 0;

    verticalAngle = 30;
    aspectRatio = 1;
    nearPlane = 10;
    farPlane = 300;

    update();
}

Grid3DWidget::~Grid3DWidget()
{

}



void Grid3DWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.drawLine(QPoint(0, this->height()/2), QPoint(this->width(), this->height()/2));
    painter.drawLine(QPoint(this->width()/2, 0), QPoint(this->width()/2, this->height()));

    paintGrid(&painter);
    return;

    QVector3D p1(-100, 50, 288-100*1.732);
    QVector3D p2(100, 50, 288-100*1.732);
    QVector3D p3(100, 250, 288);
    QVector3D p4(-100, 250, 288);

    QVector3D p5(-100, 100, 288-75*1.732);
    QVector3D p6(100, 100, 288-75*1.732);

    QVector3D p7(100, 150, 288-50*1.732);
    QVector3D p8(-100, 150, 288-50*1.732);

    QVector3D p9(100, 200, 288-25*1.732);
    QVector3D p10(-100, 200, 288-25*1.732);

    //竖线
    QVector3D p11(-50, 50, 288-100*1.732);
    QVector3D p12(-50, 250, 288);

    QVector3D p13(0, 50, 288-100*1.732);
    QVector3D p14(0, 250, 288);

    QVector3D p15(50, 50, 288-100*1.732);
    QVector3D p16(50, 250, 288);


    QMatrix4x4 mat;

    mat.translate(this->width()/2, this->height()/2, h);// this->height()/2
    mat.scale(15);
    mat.rotate(rotateX, 1, 0, 0);
    mat.rotate(rotateY, 0, 1, 0);
    mat.rotate(rotateZ, 0, 0, 1);
    mat.perspective(verticalAngle,1,nearPlane,farPlane);//45,1,0.1,1000  //aspectRatio

    QPoint pp1 = mat.map(p1).toPoint();
    QPoint pp2 = mat.map(p2).toPoint();
    QPoint pp3 = mat.map(p3).toPoint();
    QPoint pp4 = mat.map(p4).toPoint();

    QPoint pp5 = mat.map(p5).toPoint();
    QPoint pp6 = mat.map(p6).toPoint();

    QPoint pp7 = mat.map(p7).toPoint();
    QPoint pp8 = mat.map(p8).toPoint();

    QPoint pp9 = mat.map(p9).toPoint();
    QPoint pp10 = mat.map(p10).toPoint();

    //
    QPoint pp11 = mat.map(p11).toPoint();
    QPoint pp12 = mat.map(p12).toPoint();

    QPoint pp13 = mat.map(p13).toPoint();
    QPoint pp14 = mat.map(p14).toPoint();

    QPoint pp15 = mat.map(p15).toPoint();
    QPoint pp16 = mat.map(p16).toPoint();

    painter.drawLine(pp1, pp2);
    painter.drawLine(pp2, pp3);
    painter.drawLine(pp3, pp4);
    painter.drawLine(pp4, pp1);

    painter.drawLine(pp5, pp6);
    painter.drawLine(pp7, pp8);
    painter.drawLine(pp9, pp10);

    //
    painter.drawLine(pp11, pp12);
    painter.drawLine(pp13, pp14);
    painter.drawLine(pp15, pp16);

}

void Grid3DWidget::paintGrid(QPainter* painter)
{
    float step = 40;
    float maxDis = h/tan(-rotateX*M_PI/180);
    qDebug()<<"maxDis"<<maxDis<<"......................\n";
    float minDis = -400;

    float maxZVal = maxDis/cos(rotateX*M_PI/180);
    float minZVal = maxZVal-(maxDis-minDis)*cos(rotateX*M_PI/180);

    QMatrix4x4 mat;

    mat.translate(this->width()/2, this->height(), 0);//this->width()/2, this->height()/2
    mat.rotate(rotateX, 1, 0, 0);
    mat.rotate(rotateY, 0, 1, 0);
    mat.rotate(rotateZ, 0, 0, 1);

    mat.translate(0,0, -h);



//    qDebug()<<"factor: "<<factor;

    mat.scale(100*96/2.45/20);//100*96/2.45/10

    mat.perspective(verticalAngle,aspectRatio,nearPlane,farPlane); //aspectRatio

    QVector3D pFar(0, maxDis, maxZVal);
    QPointF pF = mat.map(pFar).toPointF();
    float near2FarDis = maxDis * sin(verticalAngle*M_PI/180/2) / sin((rotateX+verticalAngle/2)*M_PI/180);
    QVector3D pNear(0, maxDis-near2FarDis, maxZVal-near2FarDis*cos(rotateX*M_PI/180));
    QPointF pN = mat.map(pNear).toPointF();
    double factor = this->height()/2 / fabs(pF.y()-pN.y());



   // mat.translate(this->width()/2-pF.x(), this->height()/2-pF.y(), 0);


    for(int i=maxDis; i>=minDis; i-=step)
    {
        float zVal = maxZVal-(maxDis-i)*cos(rotateX*M_PI/180);

        QVector3D pH1(-(maxDis-minDis)/2, i, zVal);
        QPointF ph1 = mat.map(pH1).toPointF();
        QVector3D pH2((maxDis-minDis)/2, i, zVal);
        QPointF ph2 = mat.map(pH2).toPointF();

        QVector3D pV1((i-minDis)/2, maxDis, maxZVal);
        QPointF pv1 = mat.map(pV1).toPointF();
        QVector3D pV2((i-minDis)/2, minDis, minZVal);
        QPointF pv2 = mat.map(pV2).toPointF();

        QVector3D pV3(-(i-minDis)/2, maxDis, maxZVal);
        QPointF pv3 = mat.map(pV3).toPointF();
        QVector3D pV4(-(i-minDis)/2, minDis, minZVal);
        QPointF pv4 = mat.map(pV4).toPointF();

        painter->translate(this->width()/2-pF.x(), this->height()/2-pF.y());
        // H line
        painter->drawLine(ph1, ph2);
        // V line
        painter->drawLine(pv1, pv2);
        painter->drawLine(pv3, pv4);
    }

}
