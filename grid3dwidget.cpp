#include "grid3dwidget.h"
#include <math.h>

Grid3DWidget::Grid3DWidget(QWidget *parent)
    : QWidget(parent)
{
    step = 10;

    h = 200;
    rotateX = 30;
    rotateY = 0;
    rotateZ = 0;

    verticalAngle = 90;
    aspectRatio = 16/9.0;
    nearPlane = 0.1;
    farPlane = 1000;

    setMouseTracking(true);

    gridRatio = 2;



//    step = 0.1;

//    h = 1.225;
//    rotateX = 45;
//    rotateY = 0;
//    rotateZ = 0;

//    verticalAngle = 90;
//    aspectRatio = 16/9.0;
//    nearPlane = 0.1;
//    farPlane = 1000;

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

    painter.drawEllipse(ptScene2Screen(QPointF(0.1, 1)), 5, 5);
    painter.drawEllipse(ptScene2Screen(QPointF(-0.3, 1)), 5, 5);

    paintGrid(&painter);
}

void Grid3DWidget::paintGrid(QPainter* painter)
{
    //单位米
    float maxZVal = maxDis/cos(rotateX*M_PI/180);
    float minZVal = maxZVal-maxDis*cos(rotateX*M_PI/180);
    painter->drawText(QPoint(0, this->height()/2), QString::number(maxDis));
    painter->drawText(QPoint(0, this->height()), QString::number(minDis));

    painter->translate(this->width()/2-ptMiddle.x(), this->height()/2-ptMiddle.y());
    for(float i=maxDis; i>=0; i-=step)
    {
        float zVal = maxZVal-(maxDis-i)*cos(rotateX*M_PI/180);

        QVector3D pH1(-gridRatio*(maxDis-0)/2, i, zVal);
        QPointF ph1 = matrix.map(pH1).toPointF();
        QVector3D pH2(gridRatio*(maxDis-0)/2, i, zVal);
        QPointF ph2 = matrix.map(pH2).toPointF();

        // H line
        painter->drawLine(QPoint(ptMiddle.x()+(ph1.x()-ptMiddle.x())*scaleFactor, ptMiddle.y()+(ph1.y()-ptMiddle.y())*scaleFactor),
                          QPoint(ptMiddle.x()+(ph2.x()-ptMiddle.x())*scaleFactor, ptMiddle.y()+(ph2.y()-ptMiddle.y())*scaleFactor));
    }
    for(float i=0; i<=maxDis; i+=step)
    {
        //左竖线
        QVector3D pV1(gridRatio*i/2, maxDis, maxZVal);
        QPointF pv1 = matrix.map(pV1).toPointF();
        QVector3D pV2(gridRatio*i/2, 0, minZVal);
        QPointF pv2 = matrix.map(pV2).toPointF();

        //右竖线
        QVector3D pV3(-gridRatio*i/2, maxDis, maxZVal);
        QPointF pv3 = matrix.map(pV3).toPointF();
        QVector3D pV4(-gridRatio*i/2, 0, minZVal);
        QPointF pv4 = matrix.map(pV4).toPointF();

        // V line
        painter->drawLine(QPoint(ptMiddle.x()+(pv1.x()-ptMiddle.x())*scaleFactor, ptMiddle.y()+(pv1.y()-ptMiddle.y())*scaleFactor),
                          QPoint(ptMiddle.x()+(pv2.x()-ptMiddle.x())*scaleFactor, ptMiddle.y()+(pv2.y()-ptMiddle.y())*scaleFactor));
        painter->drawLine(QPoint(ptMiddle.x()+(pv3.x()-ptMiddle.x())*scaleFactor, ptMiddle.y()+(pv3.y()-ptMiddle.y())*scaleFactor),
                          QPoint(ptMiddle.x()+(pv4.x()-ptMiddle.x())*scaleFactor, ptMiddle.y()+(pv4.y()-ptMiddle.y())*scaleFactor));
    }

}

bool Grid3DWidget::prepareData()
{
    if(h <= 0 || rotateX <= 0)
        return false;


    matrix = QMatrix4x4();
    //单位米
    maxDis = h/tan(rotateX*M_PI/180);
    float maxZVal = maxDis/cos(rotateX*M_PI/180);

    //变换矩阵
    matrix.translate(this->width()/2, this->height(), 0);
    matrix.rotate(-rotateX, 1, 0, 0);
    matrix.rotate(rotateY, 0, 1, 0);
    matrix.rotate(rotateZ, 0, 0, 1);
    matrix.translate(0, 0, -h);
    matrix.perspective(verticalAngle,aspectRatio,nearPlane,farPlane);


    QVector3D pFar(0, maxDis, maxZVal);
    ptMiddle = matrix.map(pFar).toPointF();
    float near2FarDis = maxZVal * sin(verticalAngle*M_PI/180/2) / sin((rotateX+verticalAngle/2)*M_PI/180);//地面上中心点与最近点米数
    minDis = maxDis - near2FarDis;

    QVector3D pNear(0, maxDis-near2FarDis, maxZVal-near2FarDis*cos(rotateX*M_PI/180));
    QPointF pN = matrix.map(pNear).toPointF();
    scaleFactor = this->height()/2 / fabs(ptMiddle.y()-pN.y());//米->像素


    QVector3D pNormal(maxDis, maxDis, maxZVal);   //再任选一点，三点确定一个平面
    pNormal = matrix.map(pNormal);

    pFar = matrix.map(pFar);
    pNear = matrix.map(pNear);

    //平面方程（用于反算世界坐标）
    float x1 = pNormal.x();
    float y1 = pNormal.y();
    float z1 = pNormal.z();
    float x2 = pFar.x();
    float y2 = pFar.y();
    float z2 = pFar.z();
    float x3 = pNear.x();
    float y3 = pNear.y();
    float z3 = pNear.z();


    A = y1*(z2-z3) + y2*(z3-z1) + y3*(z1-z2);
    B = z1*(x2-x3) + z2*(x3-x1) + z3*(x1-x2);
    C = x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);
    D = -x1*(y2*z3-y3*z2) - x2*(y3*z1-y1*z3) - x3*(y1*z2-y2*z1);

    return true;
}

QPoint Grid3DWidget::ptScene2Screen(QPointF scenePoint)
{
    //单位米
    if(scenePoint.y() > maxDis || scenePoint.y() < 0)
        return QPoint(0, 0);

    float theltaX = this->width()/2-ptMiddle.x();    //与屏幕中心对齐
    float theltaY = this->height()/2-ptMiddle.y();

    float zVal = maxDis/cos(rotateX*M_PI/180)-(maxDis-scenePoint.y())*cos(rotateX*M_PI/180);

    QVector3D scenePt3D(-scenePoint.x(), scenePoint.y(), zVal);
    QPointF screenPt = matrix.map(scenePt3D).toPointF();
    screenPt = QPoint(ptMiddle.x()+(screenPt.x()-ptMiddle.x())*scaleFactor, ptMiddle.y()+(screenPt.y()-ptMiddle.y())*scaleFactor);
    screenPt.setX(screenPt.x() + theltaX);
    screenPt.setY(screenPt.y() + theltaY);

    return screenPt.toPoint();
}

QPointF Grid3DWidget::ptScreen2Scene(QPoint screenPt)
{
    if(screenPt.y() < this->height()/2)
        return QPoint(0, 0);

    float theltaX = this->width()/2-ptMiddle.x();    //与屏幕中心对齐
    float theltaY = this->height()/2-ptMiddle.y();
    screenPt.setX(screenPt.x() - theltaX);
    screenPt.setY(screenPt.y() - theltaY);

    QPointF newScreenPt;
    newScreenPt.setX(ptMiddle.x()+(screenPt.x()-ptMiddle.x())/scaleFactor);
    newScreenPt.setY(ptMiddle.y()+(screenPt.y()-ptMiddle.y())/scaleFactor);

    float zVal = -(D + A*newScreenPt.x() + B*newScreenPt.y()) / C;
    QVector3D scenePt3D(newScreenPt.x(), newScreenPt.y(), zVal);
    bool invertable = true;
    if(matrix.determinant() == 0)
        invertable = false;
    QMatrix4x4 invertedMatrix = matrix.inverted(&invertable);
    QVector3D originScenePt3D = invertedMatrix.map(scenePt3D);
    return QPointF(originScenePt3D.x(), originScenePt3D.y());
}

void Grid3DWidget::showEvent(QShowEvent* event)
{
    prepareData();
    update();
}

void Grid3DWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint  mousePt = event->pos();
    QPoint globalPt = event->globalPos();

    if(mousePt.y() < this->height()/2)
        return;

    scenePt = ptScreen2Scene(mousePt);
    QString ptTip = QString("(%1,%2)").arg(-scenePt.x()).arg(scenePt.y());
    QToolTip::showText(globalPt, ptTip, this, this->rect());

    update();
}

void Grid3DWidget::originPaintGrid(QPainter* painter)
{
    //单位米
    painter->drawText(QPoint(0, this->height()/2), QString::number(maxDis));
    float minDis = 0;

    float maxZVal = maxDis/cos(rotateX*M_PI/180);
    float minZVal = maxZVal-(maxDis-minDis)*cos(rotateX*M_PI/180);

    QMatrix4x4 mat;

    mat.translate(this->width()/2, this->height(), 0);
    mat.rotate(-rotateX, 1, 0, 0);
    mat.rotate(rotateY, 0, 1, 0);
    mat.rotate(rotateZ, 0, 0, 1);
    mat.translate(0, 0, -h);
    mat.perspective(verticalAngle,aspectRatio,nearPlane,farPlane);


    QVector3D pFar(0, maxDis, maxZVal);
    QPointF ptMiddle = mat.map(pFar).toPointF();
    float near2FarDis = maxZVal * sin((verticalAngle/2)*M_PI/180) / sin((rotateX+verticalAngle/2)*M_PI/180);//地面上中心点与最近点米数
    painter->drawText(QPoint(0, this->height()), QString::number(minDis));

    QVector3D pNear(0, maxDis-near2FarDis, maxZVal-near2FarDis*cos(rotateX*M_PI/180));
    QPointF pN = mat.map(pNear).toPointF();
    double factor = this->height()/2 / fabs(ptMiddle.y()-pN.y());
    //单位米->像素数

    painter->translate(this->width()/2-ptMiddle.x(), this->height()/2-ptMiddle.y());
    for(float i=maxDis; i>=minDis; i-=step)
    {
        float zVal = maxZVal-(maxDis-i)*cos(rotateX*M_PI/180);

        QVector3D pH1(-gridRatio*(maxDis-minDis)/2, i, zVal);
        QPointF ph1 = mat.map(pH1).toPointF();
        QVector3D pH2(gridRatio*(maxDis-minDis)/2, i, zVal);
        QPointF ph2 = mat.map(pH2).toPointF();

        // H line
        painter->drawLine(QPoint(ptMiddle.x()+(ph1.x()-ptMiddle.x())*factor, ptMiddle.y()+(ph1.y()-ptMiddle.y())*factor),
                          QPoint(ptMiddle.x()+(ph2.x()-ptMiddle.x())*factor, ptMiddle.y()+(ph2.y()-ptMiddle.y())*factor));
    }
    for(float i=minDis; i<=maxDis; i+=step)
    {
        //左竖线
        QVector3D pV1(gridRatio*(i-minDis)/2, maxDis, maxZVal);
        QPointF pv1 = mat.map(pV1).toPointF();
        QVector3D pV2(gridRatio*(i-minDis)/2, minDis, minZVal);
        QPointF pv2 = mat.map(pV2).toPointF();

        //右竖线
        QVector3D pV3(-gridRatio*(i-minDis)/2, maxDis, maxZVal);
        QPointF pv3 = mat.map(pV3).toPointF();
        QVector3D pV4(-gridRatio*(i-minDis)/2, minDis, minZVal);
        QPointF pv4 = mat.map(pV4).toPointF();

        // V line
        painter->drawLine(QPoint(ptMiddle.x()+(pv1.x()-ptMiddle.x())*factor, ptMiddle.y()+(pv1.y()-ptMiddle.y())*factor),
                          QPoint(ptMiddle.x()+(pv2.x()-ptMiddle.x())*factor, ptMiddle.y()+(pv2.y()-ptMiddle.y())*factor));
        painter->drawLine(QPoint(ptMiddle.x()+(pv3.x()-ptMiddle.x())*factor, ptMiddle.y()+(pv3.y()-ptMiddle.y())*factor),
                          QPoint(ptMiddle.x()+(pv4.x()-ptMiddle.x())*factor, ptMiddle.y()+(pv4.y()-ptMiddle.y())*factor));
    }
}
