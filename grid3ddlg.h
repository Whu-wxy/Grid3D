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

    Grid3DWidget* w;

    QSlider* sliderX;
    QSlider* sliderY;
    QSlider* sliderZ;
    QSlider* sliderH;

    QSlider* sliderAngle;
    QSlider* sliderRatio;
    QSlider* sliderNearPlane;
    QSlider* sliderNarPlane;



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
