#ifndef GRID3DDLG_H
#define GRID3DDLG_H

#include <QObject>
#include <QWidget>
#include <QDialog>

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>

#include <QSlider>

#include <grid3dwidget.h>


class Grid3dDlg : public QDialog
{
    Q_OBJECT
public:
    Grid3dDlg();

    Grid3DWidget* w;            //网格显示控件

    QSlider* sliderX;           //相机姿态-X
    QSlider* sliderY;           //相机姿态-Y
    QSlider* sliderZ;           //相机姿态-Z
    QSlider* sliderH;           //相机高度-H

    QSlider* sliderAngle;      //视角
    QSlider* sliderRatio;       //比例
    QSlider* sliderNearPlane;   //近平面距离值
    QSlider* sliderNarPlane;    //远平面距离值

    QLabel*  labX;
    QLabel*  labY;
    QLabel*  labZ;
    QLabel*  labH;
    QLabel*  labAngle;



protected slots:
    void    setRotateX(int);
    void    setRotateY(int);
    void    setRotateZ(int);
    void    setRotateH(int);

    void    setAngle(int);
    void    setRatio(int);
    void    setNearPlane(int);
    void    setNarPlane(int);
};

#endif // GRID3DDLG_H
