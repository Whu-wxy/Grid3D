// ///////////////////////////////////////////////////////
// Copyright (c) 2010-2020, Wuhan Beidou Marine Infor. Tech. Co., Ltd.
// All rights reserved.
//
// File name：	mapwidget.h
// Abstract ：	绘制窗口（实时视频+自绘图形内容）
// Functions：
//
// Version：	6.0.0
// Author：      wxy
// Date：        20200711
//
// //////////////////////////////////////////////////////
#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QPen>
#include <QMatrix4x4>
#include <QToolTip>
#include <QJsonObject>
#include <QtGlobal>
#include <QString>
#include <QList>
#include <QtAlgorithms>

#include "geocaculate.h"

#include "QtAV/VideoFrame.h"
#include "WidgetRenderer.h"
using namespace QtAV;
#include "infopalaisradar.h"
#include "infopalheading.h"
#include "mainmenupal.h"

class MainWidget;
class MainWindow;
class ZNDKAISObject;

class MapWidget : public WidgetRenderer
{
    Q_OBJECT

public:
    MapWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~MapWidget();

public:
// attributes:

// operations:
    void    setGridParam(double camHeight=5, double rotateX=30,     //设置格网参数（依据摄像头参数）
                         double verticalAngle=90, double step=1 ,QString color="Black");
    bool    prepareGrid();              //准备格网数据
    bool    prepareSystem2SceneMatrix();   //准备地面坐标(米)-->场景坐标(米)的转换矩阵
    // //////////控制摄像头/////////////
    void    viewLeft(double step = 2.0);    //向左看
    void    viewRigth(double step = 2.0);   //向右看
    void    viewUp(double step = 1.0);      //向上看
    void    viewDown(double step = 1.0);    //向下看
    // ////////////////////////////////

    MainWindow* getMainWindow();   //返回主窗口指针
    MainWidget* getMainWidget();    //返回主Widget指针
    // /////////////////视图子窗口//////////////////
    void        infopalAISRadar(bool bRecreate = true);      //信息子窗口(bRecreate=false,存在则不重建)
    void        infopalHeading(bool bRecreate = true);      //信息子窗口(bRecreate=false,存在则不重建)
    void        mainMenuPalette(bool bRecreate = true);      //信息子窗口(bRecreate=false,存在则不重建)
    // AIS雷达窗口
    InfoPalAISRadar* getAISRadarDialog(){ return m_pDlgAISRadar; }
    bool        createAISRadarDialog();                      //创建
    bool        destroyAISRadarDialog();                     //销毁
    // 船艏向窗口
    InfoPalHeading* getHeadingDialog(){ return m_pDlgHeading; }
    bool        createHeadingDialog();                       //创建
    bool        destroyHeadingDialog();                      //销毁
    // 底部主功能按钮窗口
    MainMenuPalette* getMainMenuPalette(){ return m_pDlgMainPal; }
    bool        createMainMenuPalette();                     //创建
    bool        destroyMainMenuPalette();                    //销毁
    // ///////////////////////////////////////////
protected:
// attributes:
    MainWidget*         m_pMainWidget;  //框架Widget
    InfoPalAISRadar*    m_pDlgAISRadar; //AIS雷达图
    InfoPalHeading*     m_pDlgHeading;  //船艏向
    MainMenuPalette*    m_pDlgMainPal;  //主按钮窗口

    bool        m_bRebuildMap;          //重建海图内容
    QPixmap*    m_pMapCanvasBMP;        //海图绘制结果缓存
// operations:
    virtual void paintEvent(QPaintEvent*);
    virtual void mouseMoveEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

    // /////////////坐标转换函数////////////////
    QPointF ptScreen2Scene(QPoint);     //屏幕坐标-->场景坐标
    QPoint  ptScene2Screen(QPointF);    //场景坐标-->屏幕坐标   正在使用
    QPoint  ptScene2ScreenMat(QPointF); //场景坐标-->屏幕坐标   未使用

    QPointF ptEarth2Scene(QPointF);     //地面坐标(经纬度)-->场景坐标(米)      未使用
    QPointF ptEarth2SceneMat(QPointF);  //地面坐标(经纬度)-->场景坐标(米)      正在使用
    QPointF ptScene2Earth(QPointF);     //场景坐标(米)-->地面坐标(经纬度)
    QPointF ptScene2EarthMat(QPointF);  //场景坐标(米)-->地面坐标(经纬度)

    QPointF ptSystem2Scene(QPointF);    //地面坐标(米)-->场景坐标(米)
    QPointF ptScene2System(QPointF);    //场景坐标(米)-->地面坐标(米)

    void    testMartix();


    // ///////////////绘制函数/////////////////
    void    paintGridBox(QPainter*);    //绘制格网
    void    drawNoAVSignal(QPainter& painter);  //绘制无信号标记
    void    paintMapScene(QPainter& painter);   //绘制海图内容（只在屏幕中心下方的内容有效）

    void    paintAISTargets(QPainter& painter); //绘制AIS目标
    void    paintAISTargetsTop(QPainter& painter); //绘制AIS目标
    void    createAISTargets(QList<ZNDKAISObject*>*);
    static bool    compareAISTargetLessThan(QPair<int, int>, QPair<int, int>);
    void    updateInfoPalAISRadar();
    // ///////////////////////////////////////
private:
// attributes:
    bool        m_bGridReady;       //格网数据准备好
    bool        m_bShowGrid;        //显示距离格网
    QString     m_color;            //网格颜色
    QVector<QPoint> m_vcHorizonLinePosL; //格网水平线的屏幕坐标(从远到近) -- 左侧
    QVector<QPoint> m_vcHorizonLinePosR; //格网水平线的屏幕坐标(从远到近) -- 右侧
    QVector<double> m_vcHorizonLineDis; //格网水平线的距离值(从远到近)

    QMatrix4x4  m_matrix;           //坐标变换矩阵
    QMatrix4x4  m_invertedMatrix;
    double      m_scaleFactor;      //米->像素
    QPointF     m_ptMiddle;         //屏幕中心点，视线中心线上的点(场景中最远点，应与屏幕中心重叠)

    qreal       m_maxDis;           //屏幕中心对应点距离摄像头多少米
    qreal       m_minDis;           //屏幕最下端对应点距离摄像头多少米

    double      m_dCamHeight;       //摄像头安装高度（距离水面距离：米）
    double      m_dCamFocus;        //摄像头焦距
    qreal       m_rotateX;          //X轴旋转（摄像头仰角）
    qreal       m_rotateY;          //Y轴旋转
    qreal       m_rotateZ;          //Z轴旋转
    qreal       m_gridRatio;        //水平方向相对垂直方向的倍率（若已知垂直50米，需要显示水平方向100米，设置2）
    qreal       m_verticalAngle;    //视场角（摄像头）
    qreal       m_nearPlane;        //近平面
    qreal       m_farPlane;         //远平面
    qreal       m_aspectRatio;      //宽高比
    double      m_dStep;            //对应现实的多少米

    //投影平面方程
    qreal       m_fA;
    qreal       m_fB;
    qreal       m_fC;
    qreal       m_fD;

    //摄像机地理信息
    double      m_dCamLat;              //摄像头位置：纬度
    double      m_dCamLon;              //摄像头位置：经度
    double      m_dCamAngle;            //摄像头角度（0~360，0°为正北）

    QTransform  m_System2SceneMatrix;   //地面坐标（米）到场景坐标转换（相机为原点）矩阵
    QTransform  m_invertedSystem2SceneMatrix; //场景坐标转换（相机为原点）到地面坐标（米）矩阵


    bool        m_bShowAISRadar;
    bool        m_bShowAISInfo;


// operations:

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void showEvent(QShowEvent *event);


protected slots:
    void    onSourceAspectRatioChanged(qreal value);
};

#endif // MAPWIDGET_H
