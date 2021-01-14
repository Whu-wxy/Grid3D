// ///////////////////////////////////////////////////////
// Copyright (c) 2010-2020, Wuhan Beidou Marine Infor. Tech. Co., Ltd.
// All rights reserved.
//
// File name：	mapwidget.cpp
// Abstract ：	绘制窗口（实时视频+自绘图形内容）
// Functions：
//
// Version：	6.0.0
// Author：      wxy
// Date：        20200711
//
// //////////////////////////////////////////////////////
#include "MapWidget.h"
#include <QDebug>
#include <iostream>
#include "mainwindow.h"
#include "mainwidget.h"
#include "enavapplication.h"
#include <QtMath>

#define CALCULATEZ
#define ROTATEX_MIN  2.0
#define ROTATEX_MAX 45.0

MapWidget::MapWidget(QWidget *parent, Qt::WindowFlags f)
    : WidgetRenderer(parent, f)
{
    setFocusPolicy(Qt::StrongFocus);
    this->setFocus();
    setBackgroundColor(QColor(200, 200, 200));

    connect(this, SIGNAL(sourceAspectRatioChanged(qreal)), this, SLOT(onSourceAspectRatioChanged(qreal)));

    m_pMainWidget = dynamic_cast<MainWidget*>(parent);
    m_pDlgAISRadar= NULL;
    m_pDlgHeading= NULL;
    m_pDlgMainPal= NULL;

    m_bGridReady = false;

    m_color = "White";

    m_bShowGrid  = true;
    m_dCamHeight = 80;
    m_rotateX    = 20;  //仰角
    m_rotateY    = 0;
    m_rotateZ    = 0;

    m_gridRatio  = 2;
    m_dCamFocus  = 1; //没有用到
    m_verticalAngle = 90;
    m_nearPlane  = 0.1;
    m_farPlane   = 5000;
    m_aspectRatio= 16.0/9;
    m_dStep      = 20;

    m_dCamLat =  30.51386;
    m_dCamLon = 114.25147;
    m_dCamAngle =360.0;  //环顾角度
    if(m_pMainWidget) m_pMainWidget->setCameraParam(m_dCamLat,m_dCamLon,m_dCamAngle,m_maxDis);

    m_bShowAISRadar = true;
    m_bShowAISInfo = true;

    m_pMapCanvasBMP = NULL;
    m_bRebuildMap = true;
    prepareGrid();  //准备网格数据
    prepareSystem2SceneMatrix();  //准备地面坐标(米)-->场景坐标(米)的转换矩阵
}

MapWidget::~MapWidget()
{
    if(NULL!=m_pMapCanvasBMP)
    {
        delete m_pMapCanvasBMP;
        m_pMapCanvasBMP = NULL;
    }
}

MainWindow* MapWidget::getMainWindow()
{
    MainWidget* pMainWidget = dynamic_cast<MainWidget*>(parent());
    if(NULL==pMainWidget) return NULL;
    else return pMainWidget->getMainWindow();
}

MainWidget* MapWidget::getMainWidget()
{
    MainWidget* pMainWidget = dynamic_cast<MainWidget*>(parent());
    return pMainWidget;
}

void MapWidget::testMartix()
{
    QMatrix4x4 matrix;
    //单位米
    m_maxDis = m_dCamHeight/qTan(qDegreesToRadians(m_rotateX));
    qreal maxZVal = m_maxDis/qCos(qDegreesToRadians(m_rotateX));

    //变换矩阵(相机)
    if(m_rotateX!=0.0) matrix.rotate(m_rotateX, 1, 0, 0); //绕X轴旋转
    if(m_rotateY!=0.0) matrix.rotate(m_rotateY, 0, 1, 0); //绕Y轴旋转
    if(m_rotateZ!=0.0) matrix.rotate(m_rotateZ, 0, 0, 1); //绕Z轴旋转
    matrix.translate(0, 0, -m_dCamHeight);
    matrix.perspective(m_verticalAngle,m_aspectRatio,m_nearPlane,m_farPlane);

    matrix = matrix/10000;
    qDebug()<<"martix: "<<matrix;


    bool invertable = true;
    if(matrix.determinant() == 0)
    {
        invertable = false;
    }
    QMatrix4x4 invertMatrix = matrix.inverted(&invertable);
   // invertMatrix = invertMatrix*10000;

    qDebug()<<"invertMatrix: "<<invertMatrix;

//    qDebug()<<"matrix multiply: "<<matrix*invertMatrix;
    // ////////////

    QVector3D pFar(0, m_maxDis, maxZVal);  //最远点的地理三维坐标（在地面上）
    QVector3D aaa = matrix.map(pFar);
    QVector3D bbb = invertMatrix.map(aaa);

   // bbb = bbb/1000;
    qDebug()<<"origin pFar: "<<pFar;
    qDebug()<<"pFar map: "<<aaa;  //QVector3D(0, 19.2958, -55.7622)
    qDebug()<<"invert pFar: "<<bbb;  //QVector3D(0, 19.2958, -55.7622)


//    D libBeidouMarineAR.so:     0.5625         0         0         0
//    D libBeidouMarineAR.so:          0  0.939693  -20.1792 0.0684054
//    D libBeidouMarineAR.so:          0   0.34202   55.4418 -0.187942
//    D libBeidouMarineAR.so:          0         0        -1         0

//    D libBeidouMarineAR.so:    1.77778         0         0         0
//    D libBeidouMarineAR.so:          0  0.939693   0.34202-1.27969e-06
//    D libBeidouMarineAR.so:          0         0         0        -1
//    D libBeidouMarineAR.so:          0   1.71007  -4.69837  -294.994
//##################################################################
//    D libBeidouMarineAR.so:  5.625e-05         0         0         0
//    D libBeidouMarineAR.so:          0 9.39693e-05 -0.00201792 6.84054e-06
//    D libBeidouMarineAR.so:          0 3.4202e-05 0.00554418  -1.87942e-05
//    D libBeidouMarineAR.so:          0         0   -0.0001         0

//    D libBeidouMarineAR.so:    17777.8         0         0         0
//    D libBeidouMarineAR.so:          0   9396.93    3420.2-0.00976465
//    D libBeidouMarineAR.so:          0         0         0    -10000
//    D libBeidouMarineAR.so:          0   17100.7  -46983.7-2.94994e+06

//1000
//100  或原矩阵/10000  QVector3D(0, 164.849, 175.428)    QVector3D(0, 164.858, 175.439)
//只原矩阵/1000                                         QVector3D(0, 165.221, 175.824)
//只原矩阵/100                                          QVector3D(0, 164.768, 175.342)
//10                                                   QVector3D(0, 164.205, 174.744)
}

// 注意：受后台视频播放自动刷新影响，会快速刷新
void MapWidget::paintEvent(QPaintEvent* event)
{
    WidgetRenderer::paintEvent(event); //视频内容（基类显示的）

    QRect client = this->geometry();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    // //////////绘制海图场景内容//////////////////
    if(m_bRebuildMap) paintMapScene(painter); //获得海图显示内容
    if(m_pMapCanvasBMP)
    {
        QRectF target(0.0, client.height() / 2.0, client.width(), client.height() / 2.0);
        QRectF source(0.0, client.height() / 2.0, client.width(), client.height() / 2.0);
        painter.drawPixmap(target,*m_pMapCanvasBMP,source);
    }
    // ///////////绘制AIS目标/////////////////////
    if(m_bShowAISInfo) paintAISTargetsTop(painter);
    // ///////////更新AISRadar显示/////////////////////
    if(m_bShowAISRadar) updateInfoPalAISRadar();
    // /////////////绘制网格//////////////////////
    if(m_bShowGrid) paintGridBox(&painter);
    // /////////////视频信号//////////////////////
    if(m_pMainWidget->getAVPlayer()==NULL) drawNoAVSignal(painter);
    else if(m_pMainWidget->getAVPlayer()->isPlaying()==false) drawNoAVSignal(painter);
    // //////////////////////////////////////////
}

void MapWidget::drawNoAVSignal(QPainter& painter)
{
    QFont font = this->font();
    font.setPointSizeF(this->font().pointSizeF()*2.0);
    QFont oldfont = painter.font();
    painter.setFont(font);
    QPen penText(m_pMainWidget->getMainWindow()->getUIColor("RedColor"));
    QPen oldpen = painter.pen();
    painter.setPen(penText);

    QRect client = this->rect();

    QPoint dspPoint(client.width()/2,client.height()/2);
    QString szDis = QString(tr("无视频信号"));
    QRect drawRect = QRect(dspPoint.x(),dspPoint.y(),100,100);
    QRect boundingRect = painter.boundingRect(drawRect,Qt::AlignLeft|Qt::AlignBottom|Qt::NoClip,szDis);   //获取绘制需要的实际大小(字体已经在总显示循环中设定)
    //调整显示位置
    dspPoint.rx() -= boundingRect.width()/2;
    dspPoint.ry() -= boundingRect.height()/2;
    boundingRect.moveBottomLeft(dspPoint);
    painter.fillRect(boundingRect,m_pMainWidget->getMainWindow()->getUIColor("WhiteColor"));
    painter.drawText(boundingRect, Qt::AlignLeft|Qt::AlignBottom, szDis );    //实际绘制
    // ////////////

    painter.setPen(oldpen);
    painter.setFont(oldfont);
}

void MapWidget::paintMapScene(QPainter& /*painter*/)
{
    QRect client = this->geometry();
    // 重建底图判定，适应分辨率改变
    if (m_pMapCanvasBMP != NULL)
    {
        if (m_pMapCanvasBMP->width() != client.width() || m_pMapCanvasBMP->height() != client.height())
        {	//需要重建
            delete m_pMapCanvasBMP;	m_pMapCanvasBMP = NULL;
        }
    }
    if (m_pMapCanvasBMP==NULL )
    {
        m_pMapCanvasBMP = new QPixmap(client.size());
        m_pMapCanvasBMP->fill(Qt::transparent); //透明模式
        if(NULL==m_pMapCanvasBMP) return;
    }
    // 擦除上次绘图内容
    QColor colorTransparent = QColor(255,0,255,0);  //透明色
    m_pMapCanvasBMP->fill(colorTransparent);
    // ////////////////////////////////////////////
    // 初始化基于Pixmap的绘图器
    QPainter bmppainter(m_pMapCanvasBMP);
    bmppainter.setRenderHint(QPainter::Antialiasing, true);
    bmppainter.setRenderHint(QPainter::SmoothPixmapTransform, true);


   // testMartix();

    //ptScreen2Scene的逆矩阵结果存在误差
    QPoint screen = ptScene2Screen(QPointF(0, m_maxDis));
    ptScreen2Scene(screen);
    //    ptScreen2Scene(QPoint(this->width()/2, this->height()/2));
    //    ptScreen2Scene(QPoint(this->width()/2, this->height()));

    //    // ///////// ptEarth2SceneMat测试 ///////////////
    //    QPointF earthPt(104.10184, 30.65884); //104.1022986174, 30.6604149398

    //    QPointF pScene = ptEarth2SceneMat(earthPt);
    //    qDebug()<<"pScene:"<<pScene.x()<<",,"<<pScene.y();
    //    QPoint pScreen0 = ptScene2Screen(pScene);

    //    QPen bluePen(Qt::blue);
    //    QPen oldPen0 = bmppainter.pen();
    //    bmppainter.setPen(bluePen);
    //    bmppainter.drawEllipse(pScreen0, 5, 5);
    //    bmppainter.setPen(oldPen0);

    //    // ///////// ptScene2EarthMat测试///////////////
    //    QPointF backEarthPt = ptScene2EarthMat(pScene);
    //    qDebug()<<"back Pos:"<<QString::number(backEarthPt.x(), 'f', 8)<<","<<QString::number(backEarthPt.y(), 'f', 8);
    //    // ///////// ptScene2EarthMat测试///////////////


    // /////// ptEarth2Scene正北方向//////////////
    QPen redPen(m_pMainWidget->getMainWindow()->getUIColor("RedColor"));
    QPen oldPen = bmppainter.pen();
    bmppainter.setPen(redPen);
    QPointF dptCamera[3];
    dptCamera[0] = QPointF(m_dCamLon,           m_dCamLat + 0.00005);
    dptCamera[1] = QPointF(m_dCamLon,           m_dCamLat + 0.00200);  //往上
    dptCamera[2] = QPointF(m_dCamLon + 0.00010, m_dCamLat + 0.00200);  //往右
    QPoint ptCamera[3];
    for(int i=0;i<3;i++){
        QPointF dptCal = ptEarth2SceneMat(dptCamera[i]);
        ptCamera[i] = ptScene2Screen(dptCal);
    }
    bmppainter.drawPolyline(ptCamera,3);


    dptCamera[0] = QPointF(m_dCamLon,           m_dCamLat - 0.00005);
    dptCamera[1] = QPointF(m_dCamLon,           m_dCamLat - 0.00200);  //往上
    dptCamera[2] = QPointF(m_dCamLon - 0.00010, m_dCamLat - 0.00200);  //往右
    for(int i=0;i<3;i++){
        QPointF dptCal = ptEarth2SceneMat(dptCamera[i]);
        ptCamera[i] = ptScene2Screen(dptCal);
    }
    bmppainter.drawPolyline(ptCamera,3);

    // /////////////// 测试船位///////////////////
    double ShipMeterX = m_dCamLon + 0.0001;
    double ShipMeterY = m_dCamLat + 0.0001;
    QPointF scenePtShip = ptEarth2SceneMat(QPointF(ShipMeterX, ShipMeterY));
    QPoint pScreenShip = ptScene2Screen(scenePtShip);

    bmppainter.drawEllipse(pScreenShip, 3, 3);

    ShipMeterX = 114.0 + 15.2435 / 60.0;
    ShipMeterY =  30.0 + 31.0939 / 60.0;
    scenePtShip = ptEarth2SceneMat(QPointF(ShipMeterX, ShipMeterY));
    pScreenShip = ptScene2Screen(scenePtShip);

    bmppainter.drawEllipse(pScreenShip, 3, 3);

    // //////////////////////////////////////////
    bmppainter.setPen(oldPen);

    // /////// ptEarth2Scene矩形测试//////////////
    QPointF dptRect[4];
    dptRect[0] = QPointF(m_dCamLon,         m_dCamLat + 0.00010);
    dptRect[1] = QPointF(m_dCamLon-0.00050, m_dCamLat + 0.00010);  //往右
    dptRect[2] = QPointF(m_dCamLon-0.00050, m_dCamLat + 0.00030);  //往下
    dptRect[3] = QPointF(m_dCamLon,         m_dCamLat + 0.00030);  //往左
    QPoint ptRect[5];
    for(int i=0;i<4;i++){
        QPointF dptCal = ptEarth2SceneMat(dptRect[i]);
        ptRect[i] = ptScene2Screen(dptCal);

    }
    ptRect[4] = ptRect[0];
    bmppainter.drawPolyline(ptRect,5);

    // ///////// ptEarth2Scene矩形测试///////////////
    //    for(int i=0;i<4;i++){
    //        QPointF dptCal = ptEarth2SceneMat(dptRect[i]);
    //        ptRect[i] = ptScene2Screen(dptCal);
    //    }
    //    ptRect[4] = ptRect[0];
    //bmppainter.drawPolyline(ptRect,5);

    // ///////// ptSystem2Scene测试//////////////////
    double camMeterX = m_dCamLon;    //104.10134
    double camMeterY = m_dCamLat;    //30.66104
    //enavEarthToMeter(camMeterX, camMeterY);

    QPointF scenePt = ptEarth2SceneMat(QPointF(camMeterX, camMeterY));
    // qDebug()<<"pt scenePt:"<<scenePt.x()<<",,"<<scenePt.y();

    QPoint pScreen = ptScene2Screen(scenePt);

    bmppainter.drawEllipse(pScreen, 10, 10);
    //   qDebug()<<"pt screen:"<<pScreen.x()<<",,"<<pScreen.y();
    // ///////// ptSystem2Scene测试//////////////////

    //qDebug()<<"#######################################";
}


void MapWidget::paintAISTargets(QPainter& painter)
{
    // 注意：不需要过滤距离和方位，下载数据的时候已经过滤了
    if(!m_pMainWidget) return;

    bdmAISTargetList* listAISTarget = m_pMainWidget->getAISObjectList();

    ///////////////////
    ZNDKAISObject* obj1 = new ZNDKAISObject();
    obj1->m_UserID = 123456;
    obj1->m_Name = "A";
    obj1->m_dSOG = 1;
    obj1->m_dCOG = 10;
    obj1->m_dLatitude = m_dCamLat + 0.001;
    obj1->m_dLongitude = m_dCamLon + 0.0005;
    obj1->m_dAISAngle = 5.1;
    obj1->m_dAISDistance = 1.1;
    ZNDKAISObject* obj2 = new ZNDKAISObject();
    obj2->m_UserID = 234567;
    obj2->m_Name = "B";
    obj2->m_dSOG = 2;
    obj2->m_dCOG = 20;
    obj2->m_dLatitude = m_dCamLat + 0.001;
    obj2->m_dLongitude = m_dCamLon - 0.0005;
    obj2->m_dAISAngle = 5.2;
    obj2->m_dAISDistance = 1.2;
    listAISTarget->append(obj1);
    listAISTarget->append(obj2);
    ///////////////////

    if(!listAISTarget) return;

    // 小于屏幕中线（Y坐标）的为超出视距的船舶
    qDebug()<<"AIS Target paint...........\n";
    int nCount = listAISTarget->count();
    for(int i=0;i<nCount;i++){
        ZNDKAISObject* pObject = listAISTarget->at(i);
        if(!pObject) continue;
        double dlat = pObject->GetLatitude();
        double dlon = pObject->GetLongitude();
        if(qAbs(dlat)<0.0||qAbs(dlat)> 90.0) continue;
        if(qAbs(dlon)<0.0||qAbs(dlon)>180.0) continue;
        qDebug()<<"AIS Target Location: "<<pObject->m_UserID<<","<<dlat<<","<<dlon<<"...........\n";
        QPointF dpoint(dlon,dlat);
        dpoint = ptEarth2SceneMat(dpoint);
        QPoint ptScreen = ptScene2Screen(dpoint);
        painter.drawEllipse(ptScreen,5,5);
        qDebug()<<"AIS Target Screen: "<<pObject->m_UserID<<","<<ptScreen.x()<<","<<ptScreen.y()<<"...........\n";

        QPoint dspPoint = ptScreen;
        // 显示neirong
        QString szDis = QString::number(pObject->m_UserID);
                szDis+= QString("\n%1°,%2kn").arg(pObject->m_dCOG,0,'f',1).arg(pObject->m_dSOG,0,'f',1);
                szDis+= QString("\n%1°,%2nm").arg(pObject->m_dAISAngle,0,'f',1).arg(pObject->m_dAISDistance,0,'f',1);
        // ///////////
        QRect drawRect = QRect(dspPoint.x(),dspPoint.y(),50,50);
        QRect boundingRect = painter.boundingRect(drawRect,Qt::AlignLeft|Qt::AlignBottom|Qt::NoClip,szDis);   //获取绘制需要的实际大小(字体已经在总显示循环中设定)

        //调整显示位置
        dspPoint.rx() -= boundingRect.width()/2;
        dspPoint.ry() -= 10;/*boundingRect.height()/2*/;
        boundingRect.moveBottomLeft(dspPoint);
        painter.fillRect(boundingRect,Qt::gray);
        painter.drawText(boundingRect, Qt::AlignLeft|Qt::AlignBottom, szDis );    //实际绘制
        painter.drawRect(boundingRect.adjusted(-1,-1,1,1));
        // ////////////
    }
}

bool MapWidget::compareAISTargetLessThan(QPair<int, int> p1, QPair<int, int> p2)
{
    return p1.second < p2.second;
}

void MapWidget::createAISTargets(QList<ZNDKAISObject*>* aisList)
{
 //   aisList->clear();
    if(aisList->count()>0) return;

    ///////////////////
    ZNDKAISObject* obj1 = new ZNDKAISObject();
    obj1->m_UserID = 123456;
    obj1->m_Name = "A";
    obj1->m_dSOG = 1;
    obj1->m_dCOG = 10;
    obj1->m_dLatitude = m_dCamLat + 0.001;
    obj1->m_dLongitude = m_dCamLon + 0.0005;
    obj1->m_dAISAngle = 5.1;
    obj1->m_dAISDistance = 1.1;
    ZNDKAISObject* obj2 = new ZNDKAISObject();
    obj2->m_UserID = 234567;
    obj2->m_Name = "B";
    obj2->m_dSOG = 2;
    obj2->m_dCOG = 20;
    obj2->m_dLatitude = m_dCamLat + 0.001;
    obj2->m_dLongitude = m_dCamLon - 0.0005;
    obj2->m_dAISAngle = 5.2;
    obj2->m_dAISDistance = 1.2;
    ZNDKAISObject* obj3 = new ZNDKAISObject();
    obj3->m_UserID = 345678;
    obj3->m_Name = "C";
    obj3->m_dSOG = 3;
    obj3->m_dCOG = 30;
    obj3->m_dLatitude = m_dCamLat + 0.001;
    obj3->m_dLongitude = m_dCamLon - 0.0009;
    obj3->m_dAISAngle = 5.3;
    obj3->m_dAISDistance = 1.3;
    ZNDKAISObject* obj4 = new ZNDKAISObject();
    obj4->m_UserID = 456789;
    obj4->m_Name = "D";
    obj4->m_dSOG = 4;
    obj4->m_dCOG = 40;
    obj4->m_dLatitude = m_dCamLat - 0.001;
    obj4->m_dLongitude = m_dCamLon - 0.0004;
    obj4->m_dAISAngle = 5.4;
    obj4->m_dAISDistance = 1.4;
    ZNDKAISObject* obj5 = new ZNDKAISObject();
    obj5->m_UserID = 567890;
    obj5->m_Name = "E";
    obj5->m_dSOG = 5;
    obj5->m_dCOG = 50;
    obj5->m_dLatitude = m_dCamLat + 0.001;
    obj5->m_dLongitude = m_dCamLon - 0.0010;
    obj5->m_dAISAngle = 5.5;
    obj5->m_dAISDistance = 1.5;
    ZNDKAISObject* obj6 = new ZNDKAISObject();
    obj6->m_UserID = 678901;
    obj6->m_Name = "F";
    obj6->m_dSOG = 6;
    obj6->m_dCOG = 60;
    obj6->m_dLatitude = m_dCamLat + 0.0006;
    obj6->m_dLongitude = m_dCamLon + 0.0009;
    obj6->m_dAISAngle = 5.6;
    obj6->m_dAISDistance = 1.6;
    ZNDKAISObject* obj7 = new ZNDKAISObject();
    obj7->m_UserID = 789012;
    obj7->m_Name = "F";
    obj7->m_dSOG = 7;
    obj7->m_dCOG = 70;
    obj7->m_dLatitude = m_dCamLat + 0.0008;
    obj7->m_dLongitude = m_dCamLon + 0.0002;
    obj7->m_dAISAngle = 5.7;
    obj7->m_dAISDistance = 1.7;
    ZNDKAISObject* obj8 = new ZNDKAISObject();
    obj8->m_UserID = 890123;
    obj8->m_Name = "G";
    obj8->m_dSOG = 8;
    obj8->m_dCOG = 80;
    obj8->m_dLatitude = m_dCamLat + 0.0003;
    obj8->m_dLongitude = m_dCamLon - 0.0002;
    obj8->m_dAISAngle = 5.8;
    obj8->m_dAISDistance = 1.8;
    ZNDKAISObject* obj9 = new ZNDKAISObject();
    obj9->m_UserID = 901234;
    obj9->m_Name = "H";
    obj9->m_dSOG = 9;
    obj9->m_dCOG = 90;
    obj9->m_dLatitude = m_dCamLat + 0.0004;
    obj9->m_dLongitude = m_dCamLon - 0.0004;
    obj9->m_dAISAngle = 5.9;
    obj9->m_dAISDistance = 1.9;
    if(aisList->isEmpty())
    {
        aisList->append(obj1);
        aisList->append(obj2);
        aisList->append(obj3);
        aisList->append(obj4);
        aisList->append(obj5);
        aisList->append(obj6);
        aisList->append(obj7);
        aisList->append(obj8);
        aisList->append(obj9);
    }
}

void MapWidget::paintAISTargetsTop(QPainter& painter)
{
    // 注意：不需要过滤距离和方位，下载数据的时候已经过滤了
    if(!m_pMainWidget) return;

    bdmAISTargetList* listAISTarget = m_pMainWidget->getAISObjectList();
    createAISTargets(listAISTarget); //如果listAISTarget为空，则装载模拟数据

    if(!listAISTarget) return;

    int aisInfoHeight = this->height()/10;
    int aisInfoWidth  = this->width() /8;

    // pen and brush
    QColor colorWhite = getMainWindow()->getUIColor("WhiteColor");
    QColor colorBlack  = getMainWindow()->getUIColor("BlackColor");
    QColor colorBlue   = getMainWindow()->getUIColor("BlueColor");
    QColor colorBG = getMainWindow()->getUIColor("LightGreyColor");
    colorBG.setAlphaF(0.3);
    colorBlack.setAlphaF(0.3);

    QBrush oldBrush = painter.brush();
    QPen oldPen = painter.pen();
    QPen bluePen(colorBlue);
    bluePen.setWidth(3);
    painter.setPen(bluePen);
    QPen whitePen(colorWhite);

    //灰色背景
    int nRightAdjust = 0;
    if(m_pDlgAISRadar)
        if(!m_pDlgAISRadar->isHidden()) nRightAdjust = m_pDlgAISRadar->width();
    painter.fillRect(QRect(0, m_pDlgHeading->height(), this->width()-nRightAdjust, aisInfoHeight+20), colorBG);

    //画点
    QList<QPair<int, int>> rightList;           //详细信息矩形中填充的信息索引,距离
    QList<QPair<int, int>> leftList;
    QMap<int, QPoint> screenPtMap;     //屏幕上画点

    int nCount = listAISTarget->count();
    int midLine = this->width()/2;
    // 小于屏幕中线（Y坐标）的为超出视距的船舶
    //qDebug()<<"AIS Target paint begin...........\n";
    for(int i=0;i<nCount;i++){
        ZNDKAISObject* pObject = listAISTarget->at(i);
        if(!pObject) continue;
        double dlat = pObject->GetLatitude();
        double dlon = pObject->GetLongitude();
        if(qAbs(dlat)<0.0||qAbs(dlat)> 90.0) continue;
        if(qAbs(dlon)<0.0||qAbs(dlon)>180.0) continue;
        //qDebug()<<"AIS Target Location: "<<pObject->m_UserID<<","<<dlat<<","<<dlon<<"...........\n";
        QPointF dpoint(dlon,dlat);
        dpoint = ptEarth2SceneMat(dpoint);
        if(dpoint.y() <= 0)
            continue;

        QPoint ptScreen = ptScene2Screen(dpoint);
        QRect visibleRect(0, this->height()/2, this->width(), this->height()/2);
        if(!visibleRect.contains(ptScreen))
            continue;

        painter.drawEllipse(ptScreen,5,5);
        //qDebug()<<"AIS Target Screen: "<<pObject->m_UserID<<","<<ptScreen.x()<<","<<ptScreen.y()<<"...........\n";
        screenPtMap.insert(i, ptScreen);

        int distance = ptScreen.x() - midLine;
        if(distance>=0)
        {
            rightList.append(QPair<int,int>(i, distance));
        }
        else
        {
            leftList.append(QPair<int,int>(i, qAbs(distance)));
        }
    }
    painter.setPen(oldPen);

    qSort(rightList.begin(), rightList.end(), MapWidget::compareAISTargetLessThan);
    qSort(leftList.begin(), leftList.end(), MapWidget::compareAISTargetLessThan);

    QFont font = qApp->font();
    font.setPixelSize(aisInfoHeight/4);
    QFont oldfont = painter.font();
    painter.setFont(font);

    //左侧绘制信息
    QPoint dspPoint(midLine, m_pDlgHeading->height()+10);
    int shiftY = aisInfoHeight/3 + 30*qMin((this->width()/2)/(aisInfoWidth+10), leftList.count());
    for(int i=0; i<leftList.count(); i++)
    {
        dspPoint.setX(dspPoint.x()-aisInfoWidth-10);
        if(dspPoint.x()<0)   //超出左边界
            break;
        //底色
        QRect boundingRect(dspPoint.x(), dspPoint.y(), aisInfoWidth, aisInfoHeight);
        painter.fillRect(boundingRect, colorBlack);

        painter.setPen(whitePen);
        // 显示neirong
        int infoIdx = leftList.at(i).first;
        ZNDKAISObject* pObject = listAISTarget->at(infoIdx);
        QString szDis = QString::number(pObject->m_UserID);
                szDis+= QString("\n%1°,%2kn").arg(pObject->m_dCOG,0,'f',1).arg(pObject->m_dSOG,0,'f',1);
                szDis+= QString("\n%1°,%2nm").arg(pObject->m_dAISAngle,0,'f',1).arg(pObject->m_dAISDistance,0,'f',1);
        // ///////////
        //文字
        painter.drawText(boundingRect, Qt::AlignLeft|Qt::AlignBottom, szDis );

        QPoint screenPt = screenPtMap.value(infoIdx);
        QPoint infoBoxPt(dspPoint.x()+aisInfoWidth/2, dspPoint.y()+aisInfoHeight);

        screenPt.setY(screenPt.y()-5);
        painter.setPen(bluePen);
        if(screenPt.x() == infoBoxPt.x())
            painter.drawLine(QLine(infoBoxPt, screenPt));
        else
        {
            QPoint connectPt1(infoBoxPt.x(), infoBoxPt.y()+shiftY-i*30);
            QPoint connectPt2(screenPt.x(), connectPt1.y());
            painter.drawLine(QLine(infoBoxPt, connectPt1));
            painter.drawLine(QLine(connectPt1, connectPt2));
            painter.drawLine(QLine(connectPt2, screenPt));
        }
    }

    //右侧绘制信息
    dspPoint.setX(midLine-aisInfoWidth);
    int nRadarWidth = 0;
    if(m_pDlgAISRadar)
        if(!m_pDlgAISRadar->isHidden()) nRadarWidth = m_pDlgAISRadar->width();
    shiftY = aisInfoHeight/3 + 30*qMin((this->width()/2-nRadarWidth)/(aisInfoWidth+10), rightList.count());
    for(int i=0; i<rightList.count(); i++)
    {
        dspPoint.setX(dspPoint.x()+aisInfoWidth+10);
        if(dspPoint.x()+aisInfoWidth>this->width())   //超出右边界
            break;
        QRect boundingRect(dspPoint.x(), dspPoint.y(), aisInfoWidth, aisInfoHeight);
        painter.fillRect(boundingRect, colorBlack);

        painter.setPen(whitePen);
        // 显示neirong
        int infoIdx = rightList.at(i).first;
        ZNDKAISObject* pObject = listAISTarget->at(infoIdx);        QString szDis = QString::number(pObject->m_UserID);
                szDis+= QString("\n%1°,%2kn").arg(pObject->m_dCOG,0,'f',1).arg(pObject->m_dSOG,0,'f',1);
                szDis+= QString("\n%1°,%2nm").arg(pObject->m_dAISAngle,0,'f',1).arg(pObject->m_dAISDistance,0,'f',1);
        // ///////////
        //文字
        painter.drawText(boundingRect, Qt::AlignLeft|Qt::AlignBottom, szDis );

        QPoint screenPt = screenPtMap.value(infoIdx);
        QPoint infoBoxPt(dspPoint.x()+aisInfoWidth/2, dspPoint.y()+aisInfoHeight);

        screenPt.setY(screenPt.y()-5);
        painter.setPen(bluePen);
        if(screenPt.x() == infoBoxPt.x())
            painter.drawLine(QLine(infoBoxPt, screenPt));
        else
        {
            QPoint connectPt1(infoBoxPt.x(), infoBoxPt.y()+shiftY-i*30);
            QPoint connectPt2(screenPt.x(), connectPt1.y());
            painter.drawLine(QLine(infoBoxPt, connectPt1));
            painter.drawLine(QLine(connectPt1, connectPt2));
            painter.drawLine(QLine(connectPt2, screenPt));
        }
    }

    painter.setPen(oldPen);
    painter.setBrush(oldBrush);
    painter.setFont(oldfont);
}

void MapWidget::updateInfoPalAISRadar()
{
    if(!m_pMainWidget && !m_pDlgAISRadar) return;

    bdmAISTargetList* listAISTarget = m_pMainWidget->getAISObjectList();
    createAISTargets(listAISTarget);

    if(!listAISTarget) return;

    m_pDlgAISRadar->setCamPosition(m_dCamLat, m_dCamLon);
    m_pDlgAISRadar->updateAISData(listAISTarget);
}

// /////////////////////Grid////////////////////
#ifdef CALCULATEZ
bool MapWidget::prepareGrid()
{
    if(m_dCamHeight <= 0 || m_rotateX <= 0){
        m_bGridReady = false;
        return false;
    }

    m_matrix = QMatrix4x4();
    //单位米
    m_maxDis = m_dCamHeight/qTan(qDegreesToRadians(m_rotateX));
    qreal maxZVal = m_maxDis/qCos(qDegreesToRadians(m_rotateX));

    //变换矩阵(相机)
    if(m_rotateX!=0.0) m_matrix.rotate(m_rotateX, 1, 0, 0); //绕X轴旋转
    if(m_rotateY!=0.0) m_matrix.rotate(m_rotateY, 0, 1, 0); //绕Y轴旋转
    if(m_rotateZ!=0.0) m_matrix.rotate(m_rotateZ, 0, 0, 1); //绕Z轴旋转
    m_matrix.translate(0, 0, -m_dCamHeight);
    m_matrix.perspective(m_verticalAngle,m_aspectRatio,m_nearPlane,m_farPlane);

    bool invertable = true;
    if(m_matrix.determinant() == 0)
    {
        invertable = false;
    }
    m_invertedMatrix = m_matrix.inverted(&invertable);
    // ////////////

    QVector3D pFar(0, m_maxDis, maxZVal);  //最远点的地理三维坐标（在地面上）
    m_ptMiddle = m_matrix.map(pFar).toPointF();  //这个点转换到摄像头画面中心点(屏幕中心在场景的位置)

    qreal near2FarDis = maxZVal * qSin(qDegreesToRadians(m_verticalAngle/2.0)) / qSin(qDegreesToRadians(m_rotateX+m_verticalAngle/2));//地面上中心点与最近点米数
    m_minDis = m_maxDis - near2FarDis;  //画面上最下端和中心点在地理坐标系上的距离米

    QVector3D pNear(0, m_maxDis-near2FarDis, maxZVal-near2FarDis*qCos(qDegreesToRadians(m_rotateX)));  //画面上最下端在地理坐标系上对应的坐标
    QPointF pN = m_matrix.map(pNear).toPointF();
    m_scaleFactor = this->height()/2 / fabs(m_ptMiddle.y()-pN.y());//米->像素 的转换系数

    QVector3D pNormal(m_maxDis, m_maxDis, maxZVal);  //再任选一点，三点确定一个平面，从屏幕坐标转换回地理坐标时需要
    pNormal = m_matrix.map(pNormal);

    pFar = m_matrix.map(pFar);
    pNear = m_matrix.map(pNear);

    //平面方程（用于反算世界坐标）
    qreal x1 = pNormal.x();
    qreal y1 = pNormal.y();
    qreal z1 = pNormal.z();
    qreal x2 = pFar.x();
    qreal y2 = pFar.y();
    qreal z2 = pFar.z();
    qreal x3 = pNear.x();
    qreal y3 = pNear.y();
    qreal z3 = pNear.z();

    m_fA = y1*(z2-z3) + y2*(z3-z1) + y3*(z1-z2);
    m_fB = z1*(x2-x3) + z2*(x3-x1) + z3*(x1-x2);
    m_fC = x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);
    m_fD = -x1*(y2*z3-y3*z2) - x2*(y3*z1-y1*z3) - x3*(y1*z2-y2*z1);

    if(!m_bGridReady) m_bGridReady = true;
    return true;
}
#endif

// ////////////////////坐标转换处理//////////////////////////////
//屏幕坐标（像素）-->场景坐标（格网坐标）
QPointF MapWidget::ptScreen2Scene(QPoint screenPt)
{
    if(screenPt.y() < this->height()/2)
        return QPoint(-1, -1);

    screenPt.setX(screenPt.x() - this->width()/2);
    screenPt.setY(screenPt.y() - this->height()/2);

    QPointF newScreenPt;
    newScreenPt.setX(m_ptMiddle.x()+screenPt.x()/m_scaleFactor);
    newScreenPt.setY(m_ptMiddle.y()+screenPt.y()/m_scaleFactor);


    qreal zVal = -(m_fD + m_fA*newScreenPt.x() + m_fB*newScreenPt.y()) / m_fC;
    QVector3D scenePt3D(newScreenPt.x(), newScreenPt.y(), zVal);

    QVector3D originScenePt3D = m_invertedMatrix.map(scenePt3D);
    return QPointF(originScenePt3D.x(), originScenePt3D.y());
}

//场景坐标-->屏幕坐标
/// ____________0, 30__________
/// ____-2,28/____|____\2,28___
/// ________/_____|_____\______
/// _______/______|______\_____
/// ______/______0,0______\____
/// ____________坐标系↑_________

#ifdef CALCULATEZ
QPoint MapWidget::ptScene2Screen(QPointF scenePt)
{
    //单位米
    qreal maxZVal = m_maxDis/qCos(qDegreesToRadians(m_rotateX));   //最远端点(m_ptMiddle)的Z值

    //求点位在摄像机Z轴（视线）上的Z值（离相机中心的距离）
    qreal zVal = maxZVal;
    if(qAbs(scenePt.y()) > m_maxDis) //大于最远点（场景中无意义了，不可视）
        zVal = maxZVal+(qAbs(scenePt.y())-m_maxDis)*qCos(qDegreesToRadians(m_rotateX));
    else
        zVal = maxZVal-(m_maxDis-qAbs(scenePt.y()))*qCos(qDegreesToRadians(m_rotateX));

    // ///////////////////////////////////////////

    QVector3D scenePt3D(-scenePt.x(), scenePt.y(), zVal);
    QPointF screenPt = m_matrix.map(scenePt3D).toPointF();
    screenPt = QPoint((screenPt.x()-m_ptMiddle.x())*m_scaleFactor, (screenPt.y()-m_ptMiddle.y())*m_scaleFactor);
    screenPt.setX(screenPt.x() + this->width()/2);
    screenPt.setY(screenPt.y() + this->height()/2);
    return screenPt.toPoint();
}
#endif

QPoint MapWidget::ptScene2ScreenMat(QPointF scenePt)
{
    QMatrix4x4 scene2camMatrix;
    scene2camMatrix.translate(-m_ptMiddle.x(),-m_ptMiddle.y());

    QVector3D scenePt3D(-scenePt.x(), scenePt.y(), 0.0);
    QVector3D cameraPt = scene2camMatrix.map(scenePt3D);

    QMatrix4x4 cam2screenMatrix;
    cam2screenMatrix.scale(m_scaleFactor,m_scaleFactor,m_scaleFactor);
    cam2screenMatrix.translate(-this->width()/2,-this->height()/2);
    QPointF screenPt = cam2screenMatrix.map(cameraPt).toPointF();

    return screenPt.toPoint();
}

//地面坐标(经纬度)-->场景坐标(米)
QPointF MapWidget::ptEarth2Scene(QPointF earthPt)
{
    //用平面距离算
    enavDPoint dpt1;
    dpt1.y = m_dCamLat;
    dpt1.x = m_dCamLon;
    enavEarthToMeter(dpt1.x,dpt1.y);
    enavDPoint dpt2;
    dpt2.y = earthPt.y();
    dpt2.x = earthPt.x();
    enavEarthToMeter(dpt2.x,dpt2.y);
    QPointF earthMeterPt;
    earthMeterPt.setX(dpt2.x-dpt1.x);
    earthMeterPt.setY(dpt2.y-dpt1.y);
    // ///////////
    return earthMeterPt;
}

//场景坐标(米)-->地面坐标(经纬度)
QPointF MapWidget::ptScene2Earth(QPointF scenePt)
{
    double sceneX = scenePt.x();
    double sceneY = scenePt.y();
    double angle2Cam = 0.0;
    if(sceneY==0 && sceneX<0)
        angle2Cam = 270;
    if(sceneY==0 && sceneX>0)
        angle2Cam = 90;
    else
        angle2Cam = qAtan(sceneX/sceneY)*180.0/M_PI;

    if(sceneX>0 && sceneY<0)
        angle2Cam += 180;
    else if(sceneX<0 && sceneY<0)
        angle2Cam += 180;

    double scenePtAngle = angle2Cam + m_dCamAngle;
    if(scenePtAngle >= 360)
        scenePtAngle -= 360;

    double distance = sqrt(pow(sceneX, 2) + pow(sceneY, 2));
    QPointF earthPt(0, 0);
    double earthX = 0.0;
    double earthY = 0.0;
    enavGCPosition(m_dCamLat, m_dCamLon, distance, scenePtAngle, earthY, earthX);
    earthPt.setX(earthX);
    earthPt.setY(earthY);

    return earthPt;
}


//场景坐标(米)-->地面坐标(经纬度)
QPointF MapWidget::ptScene2EarthMat(QPointF scenePt)
{
    QPointF systemPt = ptScene2System(scenePt);

    enavDPoint dptCal;
    dptCal.x = systemPt.x();
    dptCal.y = systemPt.y();
    enavMeterToEarth(dptCal.x, dptCal.y);

    return QPointF(dptCal.x, dptCal.y);
}

bool MapWidget::prepareSystem2SceneMatrix()
{
    double camMeterX = m_dCamLon;
    double camMeterY = m_dCamLat;

    bool res = enavEarthToMeter(camMeterX, camMeterY);
    if(res)
    {
        m_System2SceneMatrix = QTransform();

        if(m_dCamAngle!=0.0) m_System2SceneMatrix.rotate(m_dCamAngle);
        m_System2SceneMatrix.scale(1.0, -1.0);
        m_System2SceneMatrix.translate(-camMeterX, -camMeterY);

        bool invertable = true;
        if(m_System2SceneMatrix.determinant() == 0)
        {
            invertable = false;
            return false;
        }
        m_invertedSystem2SceneMatrix = m_System2SceneMatrix.inverted(&invertable);
    }

    return res;
}

//地面坐标(经纬度)-->场景坐标(米)
QPointF MapWidget::ptEarth2SceneMat(QPointF dptLonLat)
{
    enavDPoint dptCal;
    dptCal.x = dptLonLat.x();
    dptCal.y = dptLonLat.y();
    enavEarthToMeter(dptCal.x,dptCal.y);

    return ptSystem2Scene(QPointF(dptCal.x,dptCal.y));
}

//地面坐标(米)-->场景坐标
QPointF MapWidget::ptSystem2Scene(QPointF systemPt)
{
    QPointF scenePt = m_System2SceneMatrix.map(systemPt);
    return scenePt;
}

//场景坐标(米)-->地面坐标(米)
QPointF MapWidget::ptScene2System(QPointF scenePt)
{
    return m_invertedSystem2SceneMatrix.map(scenePt);
}


// ///////////////////////////////////////////////////////////////////

void MapWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(!m_bGridReady) return;

    QPoint  mousePt = event->pos();
    QPoint globalPt = event->globalPos();

    //超过屏幕1/2以上部分，距离无效
    if(mousePt.y() < this->height()/2)
    {
        update();
        return;
    }
    //屏幕1/2以下部分，距离有效
    QPointF scenePt = ptScreen2Scene(mousePt);
    if(scenePt.x() == -1)
        return;
    QString ptTip = QString("(%1,%2)").arg(-scenePt.x()).arg(scenePt.y());
    QToolTip::showText(QPoint(0,0)/*globalPt*/, ptTip, this, this->rect());
    qDebug()<<"Mouse location: "<<ptTip<<".........................\n";

    update();
    // //////////////////////
}

void MapWidget::resizeEvent(QResizeEvent* e)
{
    prepareGrid();
    prepareSystem2SceneMatrix();
    m_bRebuildMap = true;
    infopalHeading(false);  //调整窗口
    infopalAISRadar(false); //调整窗口
    mainMenuPalette(false);
    WidgetRenderer::resizeEvent(e);
}

#ifdef CALCULATEZ
void MapWidget::paintGridBox(QPainter* painter)
{
    QPen oldPen = painter->pen();

    QPen newPen;
    if(m_color == "Black")
        newPen.setColor(Qt::black);
    else if(m_color == "White")
        newPen.setColor(Qt::white);
    else
        newPen.setColor(Qt::red);
    painter->setPen(newPen);

    //屏幕中心十字分界线
    painter->drawLine(QPoint(0, this->height()/2), QPoint(this->width(), this->height()/2));
    painter->drawLine(QPoint(this->width()/2, 0), QPoint(this->width()/2, this->height()));

    //距离指示（远&近）：单位米
    qreal maxZVal = m_maxDis/qCos(qDegreesToRadians(m_rotateX));
    qreal minZVal = maxZVal-m_maxDis*qCos(qDegreesToRadians(m_rotateX));

    painter->drawText(QPoint(0, this->height()/2), QString::number(m_maxDis,'f',1));
    painter->drawText(QPoint(0, this->height()), QString::number(m_minDis,'f',1));

    //调整原点到屏幕中心
    painter->translate(this->width()/2, this->height()/2);
    //绘制水平线
    m_vcHorizonLinePosL.clear();
    m_vcHorizonLinePosR.clear();
    m_vcHorizonLineDis.clear();
    // ////////////
    for(qreal i=m_maxDis; i>=0; i-=m_dStep)
    {
        // 画整数级距离线
        int nIntDis   = (int)(i / m_dStep);
        qreal fIntDis = (qreal)nIntDis * m_dStep;
        // ////////////

        qreal zVal = maxZVal-(m_maxDis-fIntDis)*qCos(qDegreesToRadians(m_rotateX));

        QVector3D pH1(-m_maxDis*m_gridRatio, fIntDis, zVal);
        QPointF   ph1 = m_matrix.map(pH1).toPointF();
        QVector3D pH2(m_maxDis*m_gridRatio, fIntDis, zVal);
        QPointF   ph2 = m_matrix.map(pH2).toPointF();

        // H line
        double rightX= (ph1.x()-m_ptMiddle.x())*m_scaleFactor;
        double rightY= (ph1.y()-m_ptMiddle.y())*m_scaleFactor;
        double leftX = (ph2.x()-m_ptMiddle.x())*m_scaleFactor;
        double leftY = (ph2.y()-m_ptMiddle.y())*m_scaleFactor;
        painter->drawLine(QPoint((int)leftX, (int)leftY), QPoint((int)rightX, (int)rightY));

        // 记录屏幕位置和距离
        m_vcHorizonLinePosL.append(QPoint((int)leftX, (int)leftY));
        m_vcHorizonLinePosR.append(QPoint((int)rightX,(int)rightY));
        m_vcHorizonLineDis.append(fIntDis);
        // ////////////////
    }
    //绘制竖线
    for(qreal i=0; i<=m_maxDis*m_gridRatio; i+=m_dStep)
    {
        // 画整数级距离线
        int nIntDis   = (int)(i / m_dStep);
        qreal fIntDis = (qreal)nIntDis * m_dStep;
        // ////////////
        //左竖线
        QVector3D pV1(fIntDis, m_maxDis, maxZVal);
        QPointF pv1 = m_matrix.map(pV1).toPointF();
        QVector3D pV2(fIntDis, 0, minZVal);
        QPointF pv2 = m_matrix.map(pV2).toPointF();

        //右竖线
        QVector3D pV3(-fIntDis, m_maxDis, maxZVal);
        QPointF pv3 = m_matrix.map(pV3).toPointF();
        QVector3D pV4(-fIntDis, 0, minZVal);
        QPointF pv4 = m_matrix.map(pV4).toPointF();

        // V line
        painter->drawLine(QPoint((pv1.x()-m_ptMiddle.x())*m_scaleFactor, (pv1.y()-m_ptMiddle.y())*m_scaleFactor),
                          QPoint((pv2.x()-m_ptMiddle.x())*m_scaleFactor, (pv2.y()-m_ptMiddle.y())*m_scaleFactor));
        painter->drawLine(QPoint((pv3.x()-m_ptMiddle.x())*m_scaleFactor, (pv3.y()-m_ptMiddle.y())*m_scaleFactor),
                          QPoint((pv4.x()-m_ptMiddle.x())*m_scaleFactor, (pv4.y()-m_ptMiddle.y())*m_scaleFactor));
    }
    // 绘制网格线距离
    if(m_vcHorizonLineDis.count()>0){
        for(int i=1;i<m_vcHorizonLineDis.count();i++)
        {
            if(i%2==0) continue; //间隔画距离标记

            double dis = m_vcHorizonLineDis.at(i);
            QString szDis = QString::number(dis,'f',0);
            QPoint dspPoint = QPoint(-this->width()/2 + 10,  m_vcHorizonLinePosL.at(i).y());
            QRect drawRect = QRect(dspPoint.x(),dspPoint.y(),100,100);
            QRect boundingRect = painter->boundingRect(drawRect,Qt::AlignLeft|Qt::AlignBottom|Qt::NoClip,szDis);   //获取绘制需要的实际大小(字体已经在总显示循环中设定)
            //调整到居中显示
            dspPoint.ry() += boundingRect.height()/2;
            boundingRect.moveBottomLeft(dspPoint);
            painter->fillRect(boundingRect,Qt::gray);
            painter->drawText(boundingRect, Qt::AlignLeft|Qt::AlignBottom, szDis );    //实际绘制
            // ////////////
        }
    }
    // ////////////
    painter->setPen(oldPen);
    painter->translate(-this->width()/2, -this->height()/2);
}
#endif

void MapWidget::setGridParam(double camHeight, double rotateX,
                             double verticalAngle, double step, QString color)
{
    m_bGridReady = false;
    if(!m_bShowGrid)
        return;
    if(rotateX <= 0 || verticalAngle <= 0 || step <= 0 || camHeight <= 0)
        return;
    m_dCamHeight = camHeight;
    m_rotateX = rotateX;
    m_verticalAngle = verticalAngle;
    m_dStep = step;
    m_color = color;

    //
    m_rotateY = 0;
    m_rotateZ = 0;
    m_nearPlane = 0.1;
    m_farPlane = 5000;
    m_aspectRatio = 16.0/9;

    m_gridRatio = 4;

    if(prepareGrid() && prepareSystem2SceneMatrix())
    {
        m_bGridReady = true;
        update();
    }
}

void MapWidget::keyPressEvent(QKeyEvent *event)
{
    int nKey = event->key();
    switch(nKey)
    {
    case Qt::Key_Right:
        viewRigth(2.0);
        break;
    case Qt::Key_Left:
        viewLeft(2.0);
        break;
    case Qt::Key_Up:
        viewUp(1.0);
        break;
    case Qt::Key_Down:
        viewDown(1.0);
        break;
    case Qt::Key_F12:
        infopalAISRadar();
        break;
    default: break;
    }
    WidgetRenderer::keyPressEvent(event);
}

void MapWidget::mousePressEvent(QMouseEvent *event)
{
    WidgetRenderer::mousePressEvent(event);
}

void MapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QRect client = this->rect();
    QPoint mousePos = event->pos();
    if(mousePos.y()<client.height()/2) return;
    if(mousePos.x()<client.width()/2) viewLeft(2.0);
    else viewRigth(2.0);

    WidgetRenderer::mouseReleaseEvent(event);
}

void MapWidget::viewLeft(double step/* = 5.0*/)
{
    m_dCamAngle -= step;
    if(m_dCamAngle>360.0) m_dCamAngle -= 360.0;
    else if(m_dCamAngle<0.0) m_dCamAngle += 360.0;
    prepareGrid();  //准备网格数据
    prepareSystem2SceneMatrix();  //准备地面坐标(米)-->场景坐标(米)的转换矩阵
    if(m_pDlgAISRadar) m_pDlgAISRadar->setHeadingAngle(m_dCamAngle);
    m_bRebuildMap = true;
    if(m_pDlgHeading) m_pDlgHeading->headingChanged(m_dCamAngle);
    if(m_pMainWidget) m_pMainWidget->setCameraParam(m_dCamLat,m_dCamLon,m_dCamAngle,m_maxDis);
    update();
}

void MapWidget::viewRigth(double step/* = 5.0*/)
{
    m_dCamAngle += step;
    if(m_dCamAngle>360.0) m_dCamAngle -= 360.0;
    else if(m_dCamAngle<0.0) m_dCamAngle += 360.0;
    prepareGrid();  //准备网格数据
    prepareSystem2SceneMatrix();  //准备地面坐标(米)-->场景坐标(米)的转换矩阵
    if(m_pDlgAISRadar) m_pDlgAISRadar->setHeadingAngle(m_dCamAngle);
    m_bRebuildMap = true;
    if(m_pDlgHeading) m_pDlgHeading->headingChanged(m_dCamAngle);
    if(m_pMainWidget) m_pMainWidget->setCameraParam(m_dCamLat,m_dCamLon,m_dCamAngle,m_maxDis);
    update();
}

void MapWidget::viewUp(double step/* = 1.0*/)
{
    m_rotateX -= step;
    if(m_rotateX>ROTATEX_MAX) m_rotateX = ROTATEX_MAX;
    else if(m_rotateX<ROTATEX_MIN) m_rotateX = ROTATEX_MIN;
    // 调整距离标尺线间距
    if(m_rotateX<5.0) m_dStep = 100.0;
    else if(m_rotateX<10.0) m_dStep = 40.0;
    else if(m_rotateX<20.0) m_dStep = 20.0;
    else m_dStep = 10.0;
    // ///////////////
    prepareGrid();  //准备网格数据
    prepareSystem2SceneMatrix();  //准备地面坐标(米)-->场景坐标(米)的转换矩阵
    if(m_pDlgAISRadar){
        m_pDlgAISRadar->setVisibleDistance(m_maxDis);
    }
    m_bRebuildMap = true;
    update();
}

void MapWidget::viewDown(double step/* = 1.0*/)
{
    m_rotateX += step;
    if(m_rotateX>ROTATEX_MAX) m_rotateX = ROTATEX_MAX;
    else if(m_rotateX<ROTATEX_MIN) m_rotateX = ROTATEX_MIN;
    // 调整距离标尺线间距
    if(m_rotateX<5.0) m_dStep = 100.0;
    else if(m_rotateX<10.0) m_dStep = 40.0;
    else if(m_rotateX<20.0) m_dStep = 20.0;
    else m_dStep = 10.0;
    // ///////////////
    prepareGrid();  //准备网格数据
    prepareSystem2SceneMatrix();  //准备地面坐标(米)-->场景坐标(米)的转换矩阵
    if(m_pDlgAISRadar){
        m_pDlgAISRadar->setVisibleDistance(m_maxDis);
    }
    m_bRebuildMap = true;
    update();
}

//////////////先透视在平移旋转，Z=0///////////////////////
#ifndef CALCULATEZ
bool MapWidget::prepareGrid()
{
    if(m_dCamHeight <= 0 || m_rotateX <= 0){
        m_bGridReady = false;
        return false;
    }

    m_matrix = QMatrix4x4();
    //单位米
    m_maxDis = m_dCamHeight/tan(m_rotateX*M_PI/180);
    float maxZVal = m_maxDis/cos(m_rotateX*M_PI/180);

    //变换矩阵
    m_matrix.perspective(m_verticalAngle,m_aspectRatio,m_nearPlane,m_farPlane);
    m_matrix.translate(0, 0, m_dCamHeight);
    if(m_rotateX!=0.0) m_matrix.rotate(m_rotateX, 1, 0, 0); //绕X轴旋转
    if(m_rotateY!=0.0) m_matrix.rotate(m_rotateY, 0, 1, 0); //绕Y轴旋转
    if(m_rotateZ!=0.0) m_matrix.rotate(m_rotateZ, 0, 0, 1); //绕Z轴旋转

    QVector3D pFar(0, m_maxDis, 0);  //最远点的地理三维坐标（在地面上）

    m_ptMiddle = m_matrix.map(pFar).toPointF();  //这个点转换到摄像头画面中心点
    float near2FarDis = maxZVal * sin(m_verticalAngle*M_PI/180/2) / sin((m_rotateX+m_verticalAngle/2)*M_PI/180);//地面上中心点与最近点米数
    m_minDis = m_maxDis - near2FarDis;  //画面上最下端和中心点在地理坐标系上的距离米

    QVector3D pNear(0, m_maxDis-near2FarDis, 0);  //画面上最下端在地理坐标系上对应的坐标
    QPointF pN = m_matrix.map(pNear).toPointF();
    m_scaleFactor = this->height()/2 / fabs(m_ptMiddle.y()-pN.y());//米->像素 的转换系数

    QVector3D pNormal(m_maxDis, m_maxDis, 0);  //再任选一点，三点确定一个平面，从屏幕坐标转换回地理坐标时需要

    pNormal = m_matrix.map(pNormal);

    pFar = m_matrix.map(pFar);
    pNear = m_matrix.map(pNear);

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

    m_fA = y1*(z2-z3) + y2*(z3-z1) + y3*(z1-z2);
    m_fB = z1*(x2-x3) + z2*(x3-x1) + z3*(x1-x2);
    m_fC = x1*(y2-y3) + x2*(y3-y1) + x3*(y1-y2);
    m_fD = -x1*(y2*z3-y3*z2) - x2*(y3*z1-y1*z3) - x3*(y1*z2-y2*z1);

    if(!m_bGridReady) m_bGridReady = true;
    return true;
}


QPoint MapWidget::ptScene2Screen(QPointF scenePt)
{
    qDebug()<<"scenePt: "<<scenePt;
    QVector3D scenePt3D(-scenePt.x(), scenePt.y(), 0);
    QPointF screenPt = m_matrix.map(scenePt3D).toPointF();
    qDebug()<<"screenPt: "<<screenPt;

    screenPt = QPoint((screenPt.x()-m_ptMiddle.x())*m_scaleFactor, (screenPt.y()-m_ptMiddle.y())*m_scaleFactor);
    qDebug()<<"screenPt2: "<<screenPt;


    screenPt.setX(screenPt.x() + this->width()/2);
    screenPt.setY(screenPt.y() + this->height()/2);

    return screenPt.toPoint();
}


void MapWidget::paintGridBox(QPainter* painter)
{
    QPen oldPen = painter->pen();

    QPen newPen;
    if(m_color == "Black")
        newPen.setColor(Qt::black);
    else if(m_color == "White")
        newPen.setColor(Qt::white);
    else
        newPen.setColor(Qt::red);
    painter->setPen(newPen);

    //屏幕中心十字分界线
    painter->drawLine(QPoint(0, this->height()/2), QPoint(this->width(), this->height()/2));
    painter->drawLine(QPoint(this->width()/2, 0), QPoint(this->width()/2, this->height()));

    painter->drawText(QPoint(0, this->height()/2), QString::number(m_maxDis,'f',1));
    painter->drawText(QPoint(0, this->height()), QString::number(m_minDis,'f',1));

    //调整原点到屏幕中心
    painter->translate(this->width()/2, this->height()/2);
    //绘制水平线
    m_vcHorizonLinePosL.clear();
    m_vcHorizonLinePosR.clear();
    m_vcHorizonLineDis.clear();
    // ////////////
    for(float i=m_maxDis; i>=0; i-=m_dStep)
    {
        // 画整数级距离线
        int nIntDis   = (int)(i / m_dStep);
        float fIntDis = (float)nIntDis * m_dStep;
        // ////////////

        QVector3D pH1(-m_maxDis*m_gridRatio, fIntDis, 0);
        QPointF   ph1 = m_matrix.map(pH1).toPointF();
        QVector3D pH2(m_maxDis*m_gridRatio, fIntDis, 0);
        QPointF   ph2 = m_matrix.map(pH2).toPointF();

        // H line
        double rightX= (ph1.x()-m_ptMiddle.x())*m_scaleFactor;
        double rightY= (ph1.y()-m_ptMiddle.y())*m_scaleFactor;
        double leftX = (ph2.x()-m_ptMiddle.x())*m_scaleFactor;
        double leftY = (ph2.y()-m_ptMiddle.y())*m_scaleFactor;
        painter->drawLine(QPoint((int)leftX, (int)leftY), QPoint((int)rightX, (int)rightY));

        // 记录屏幕位置和距离
        m_vcHorizonLinePosL.append(QPoint((int)leftX, (int)leftY));
        m_vcHorizonLinePosR.append(QPoint((int)rightX,(int)rightY));
        m_vcHorizonLineDis.append(fIntDis);
        // ////////////////
    }
    //绘制竖线
    for(float i=0; i<=m_maxDis*m_gridRatio; i+=m_dStep)
    {
        // 画整数级距离线
        int nIntDis   = (int)(i / m_dStep);
        float fIntDis = (float)nIntDis * m_dStep;
        // ////////////
        //左竖线
        QVector3D pV1(fIntDis, m_maxDis, 0);
        QPointF pv1 = m_matrix.map(pV1).toPointF();
        QVector3D pV2(fIntDis, 0, 0);
        QPointF pv2 = m_matrix.map(pV2).toPointF();

        //右竖线
        QVector3D pV3(-fIntDis, m_maxDis, 0);
        QPointF pv3 = m_matrix.map(pV3).toPointF();
        QVector3D pV4(-fIntDis, 0, 0);
        QPointF pv4 = m_matrix.map(pV4).toPointF();

        // V line
        painter->drawLine(QPoint((pv1.x()-m_ptMiddle.x())*m_scaleFactor, (pv1.y()-m_ptMiddle.y())*m_scaleFactor),
                          QPoint((pv2.x()-m_ptMiddle.x())*m_scaleFactor, (pv2.y()-m_ptMiddle.y())*m_scaleFactor));
        painter->drawLine(QPoint((pv3.x()-m_ptMiddle.x())*m_scaleFactor, (pv3.y()-m_ptMiddle.y())*m_scaleFactor),
                          QPoint((pv4.x()-m_ptMiddle.x())*m_scaleFactor, (pv4.y()-m_ptMiddle.y())*m_scaleFactor));
    }
    // 绘制网格线距离
    if(m_vcHorizonLineDis.count()>0){
        for(int i=1;i<m_vcHorizonLineDis.count();i++)
        {
            if(i%2==0) continue; //间隔画距离标记

            double dis = m_vcHorizonLineDis.at(i);
            QString szDis = QString::number(dis,'f',0);
            QPoint dspPoint = QPoint(-this->width()/2 + 10,  m_vcHorizonLinePosL.at(i).y());
            QRect drawRect = QRect(dspPoint.x(),dspPoint.y(),100,100);
            QRect boundingRect = painter->boundingRect(drawRect,Qt::AlignLeft|Qt::AlignBottom|Qt::NoClip,szDis);   //获取绘制需要的实际大小(字体已经在总显示循环中设定)
            //调整到居中显示
            dspPoint.ry() += boundingRect.height()/2;
            boundingRect.moveBottomLeft(dspPoint);
            painter->fillRect(boundingRect,Qt::gray);
            painter->drawText(boundingRect, Qt::AlignLeft|Qt::AlignBottom, szDis );    //实际绘制
            // ////////////
        }
    }
    // ////////////
    painter->setPen(oldPen);
    painter->translate(-this->width()/2, -this->height()/2);
}
#endif

// 角度+距离的ptEarth2Scene
//QPointF MapWidget::ptEarth2Scene(QPointF earthPt)
//{
//    //用平面距离算
//    double angle = enavEllipsoidAzimuth(m_dCamLat, m_dCamLon, earthPt.y(), earthPt.x());
//    double angle2Cam = angle-m_dCamAngle;

//    double distance = enavApproxDistance(m_dCamLat, m_dCamLon, earthPt.y(), earthPt.x());

//    QPointF earthMeterPt;
//    earthMeterPt.setX(1000*distance*sin(angle2Cam*M_PI/180));
//    earthMeterPt.setY(1000*distance*cos(angle2Cam*M_PI/180));
//    // ///////////
//    return earthMeterPt;
//}


void MapWidget::showEvent(QShowEvent *event)
{
    WidgetRenderer::showEvent(event);
}

// ////////////////////////信息子窗口//////////////////////
// AIS雷达图
void MapWidget::infopalAISRadar(bool bRecreate/* = true*/)
{
    if(m_pDlgAISRadar!=NULL)
    {
        if(bRecreate){
            //destroyAISRadarDialog();
            if(m_pDlgAISRadar->isHidden()){
                m_pDlgAISRadar->show();
            }
            else m_pDlgAISRadar->hide();
        }
        if(!m_pDlgAISRadar->isHidden())
        {
            // 显示到合适位置
            QRect client = this->rect();
            QSize dlgsize= m_pDlgAISRadar->sizeHint();
            int nWidth = qMax(client.width()*1/6,dlgsize.width());
            int nHeight= nWidth;
            m_pDlgAISRadar->setFixedSize(nWidth,nHeight);
            dlgsize = m_pDlgAISRadar->geometry().size();
            QPoint dlgpos(client.right()-dlgsize.width(), client.top()+5);
            m_pDlgAISRadar->move(dlgpos);
        }
    }
    else createAISRadarDialog();
}

bool MapWidget::createAISRadarDialog()
{
    if(m_pDlgAISRadar!=NULL) return true;

    m_pDlgAISRadar = new InfoPalAISRadar(this);
    if(NULL==m_pDlgAISRadar) return false;
    QColor bkcolor = Qt::black;
    m_pDlgAISRadar->setBackgroundColor(bkcolor);
    // 显示到合适位置
    QRect client = this->rect();
    QSize dlgsize= m_pDlgAISRadar->sizeHint();
    int nWidth = qMax(client.width()*1/6,dlgsize.width());
    int nHeight= nWidth;
    m_pDlgAISRadar->setFixedSize(nWidth,nHeight);
    dlgsize = m_pDlgAISRadar->geometry().size();
    QPoint dlgpos(client.right()-dlgsize.width(), client.top()+5);
    m_pDlgAISRadar->move(dlgpos);
    m_pDlgAISRadar->show();
    // 刷数据
    m_pDlgAISRadar->setMaxDistance(NM2M(0.2)/*2*/);
    m_pDlgAISRadar->setVisibleDistance(m_maxDis);
    // ////////////
    return true;
}

bool MapWidget::destroyAISRadarDialog()
{
    if(m_pDlgAISRadar!=NULL)
    {
        delete m_pDlgAISRadar;
        m_pDlgAISRadar = NULL;
        return true;
    }
    else return false;
}

// 船艏向指示窗
void MapWidget::infopalHeading(bool bRecreate/* = true*/)
{
    if(m_pDlgHeading!=NULL)
    {
        if(bRecreate) destroyHeadingDialog();
        else{
            QRect client = this->rect();
            QSize dlgsize = m_pDlgHeading->geometry().size();
            int nWidth = qMax(client.width() *3/5,dlgsize.width());
            int nHeight= qMax(client.height()*1/15,dlgsize.height());
            m_pDlgHeading->setFixedSize(nWidth,nHeight);
            QPoint dlgpos(client.center().x()-dlgsize.width()/2, 0);
            m_pDlgHeading->move(dlgpos);
        }
    }
    else createHeadingDialog();
}

// 船艏向指示窗
bool MapWidget::createHeadingDialog()
{
    if(m_pDlgHeading!=NULL) return true;

    m_pDlgHeading = new InfoPalHeading(this);
    if(NULL==m_pDlgHeading) return false;
    QColor bkcolor = Qt::black;
    m_pDlgHeading->setBackgroundColor(bkcolor);
    // 显示到合适位置
    QRect client = this->rect();
    QSize dlgsize= m_pDlgHeading->sizeHint();
    int nWidth = qMax(client.width() *3/5,dlgsize.width());
    int nHeight= qMax(client.height()*1/15,dlgsize.height());
    m_pDlgHeading->setFixedSize(nWidth,nHeight);
    dlgsize = m_pDlgHeading->geometry().size();
    QPoint dlgpos(client.center().x()-dlgsize.width()/2, 0);
    m_pDlgHeading->move(dlgpos);
    m_pDlgHeading->show();
    // 刷数据

    // ////////////
    return true;
}

bool MapWidget::destroyHeadingDialog()
{
    if(m_pDlgHeading!=NULL)
    {
        delete m_pDlgHeading;
        m_pDlgHeading = NULL;
        return true;
    }
    else return false;
}

// 主控制按钮
void MapWidget::mainMenuPalette(bool bRecreate/* = true*/)
{
    if(m_pDlgMainPal!=NULL)
    {
        if(bRecreate) destroyMainMenuPalette();
        else{
            QRect client = this->rect();
            QSize dlgsize = m_pDlgMainPal->geometry().size();
            int nWidth = qMax(client.width() *1/5,dlgsize.width());
            int nHeight= qMax(client.height()*1/16,dlgsize.height());
            m_pDlgMainPal->setFixedSize(nWidth,nHeight);
            QPoint dlgpos(client.center().x()-dlgsize.width()/2, client.bottom()-dlgsize.height());
            m_pDlgMainPal->move(dlgpos);
        }
    }
    else createMainMenuPalette();
}

// 船艏向指示窗
bool MapWidget::createMainMenuPalette()
{
    if(m_pDlgMainPal!=NULL) return true;

    m_pDlgMainPal = new MainMenuPalette(this);
    if(NULL==m_pDlgMainPal) return false;
    QColor bkcolor = Qt::black;
    m_pDlgMainPal->setBackgroundColor(bkcolor);
    // 显示到合适位置
    QRect client = this->rect();
    QSize dlgsize= m_pDlgMainPal->sizeHint();
    int nWidth = qMax(client.width() *1/5,dlgsize.width());
    int nHeight= qMax(client.height()*1/16,dlgsize.height());
    m_pDlgMainPal->setFixedSize(nWidth,nHeight);
    dlgsize = m_pDlgMainPal->geometry().size();
    QPoint dlgpos(client.center().x()-dlgsize.width()/2, 0);
    m_pDlgMainPal->move(dlgpos);
    m_pDlgMainPal->show();
    // 刷数据

    // ////////////
    return true;
}

bool MapWidget::destroyMainMenuPalette()
{
    if(m_pDlgMainPal!=NULL)
    {
        delete m_pDlgMainPal;
        m_pDlgMainPal = NULL;
        return true;
    }
    else return false;
}

// ////////////////////


void MapWidget::onSourceAspectRatioChanged(qreal value)
{
    qDebug()<<"onSourceAspectRatioChanged(qreal): "<<value;
}
// ///////////////////////////////////////////////////////
