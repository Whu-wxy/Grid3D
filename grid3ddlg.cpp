#include "grid3ddlg.h"
#include <QDebug>

Grid3dDlg::Grid3dDlg()
{
    resize(700, 700);

    w = new Grid3DWidget(this);

    sliderX = new QSlider(this);
    sliderY = new QSlider(this);
    sliderZ = new QSlider(this);
    sliderH = new QSlider(this);

    sliderAngle = new QSlider(this);
    sliderRatio = new QSlider(this);
    sliderNearPlane = new QSlider(this);
    sliderNarPlane = new QSlider(this);

    sliderX->setTickInterval(1);
    sliderX->setRange(0, 90);
    sliderX->setValue(30);
    sliderX->setSliderPosition(30);

    sliderY->setTickInterval(1);
    sliderY->setRange(-180, 180);
    sliderY->setValue(0);
    sliderY->setSliderPosition(0);

    sliderZ->setTickInterval(1);
    sliderZ->setRange(-180, 180);
    sliderZ->setValue(0);
    sliderZ->setSliderPosition(0);

    sliderH->setTickInterval(1);
    sliderH->setRange(0, 500);
    sliderH->setValue(144);
    sliderH->setSliderPosition(144);
    // //////////////////////////
    sliderAngle->setTickInterval(1);
    sliderAngle->setRange(5, 180);
    sliderAngle->setValue(30);
    sliderAngle->setSliderPosition(30);

    sliderRatio->setTickInterval(1);
    sliderRatio->setRange(1, 30);
    sliderRatio->setValue(10);
    sliderRatio->setSliderPosition(10);

    sliderNearPlane->setTickInterval(1);
    sliderNearPlane->setRange(0, 20);
    sliderNearPlane->setValue(10);
    sliderNearPlane->setSliderPosition(10);

    sliderNarPlane->setTickInterval(1);
    sliderNarPlane->setRange(0, 5000);
    sliderNarPlane->setValue(300);
    sliderNarPlane->setSliderPosition(300);

    QHBoxLayout* mainLay = new QHBoxLayout(this);

    QGridLayout* LeftLay = new QGridLayout;
    QLabel* xLab = new QLabel("X");
    QLabel* yLab = new QLabel("Y");
    QLabel* zLab = new QLabel("Z");
    QLabel* hLab = new QLabel("H");
    LeftLay->addWidget(xLab, 0, 0, 1, 1, Qt::AlignCenter);
    LeftLay->addWidget(yLab, 0, 1, 1, 1, Qt::AlignCenter);
    LeftLay->addWidget(zLab, 0, 2, 1, 1, Qt::AlignCenter);
    LeftLay->addWidget(hLab, 0, 3, 1, 1, Qt::AlignCenter);
    LeftLay->addWidget(sliderX, 1, 0, 1, 1);
    LeftLay->addWidget(sliderY, 1, 1, 1, 1);
    LeftLay->addWidget(sliderZ, 1, 2, 1, 1);
    LeftLay->addWidget(sliderH, 1, 3, 1, 1);

    QGridLayout* RightLay = new QGridLayout;
    QLabel* perspLab = new QLabel("透视变换");
    RightLay->addWidget(perspLab, 0, 0, 1, 4, Qt::AlignCenter);
    RightLay->addWidget(sliderAngle, 1, 0, 1, 1);
    RightLay->addWidget(sliderRatio, 1, 1, 1, 1);
    RightLay->addWidget(sliderNearPlane, 1, 2, 1, 1);
    RightLay->addWidget(sliderNarPlane, 1, 3, 1, 1);

    mainLay->addLayout(LeftLay);
    mainLay->addWidget(w);
    mainLay->addLayout(RightLay);

    connect(sliderX, SIGNAL(valueChanged(int)),this,SLOT(setRotateX(int)));
    connect(sliderY, SIGNAL(valueChanged(int)),this,SLOT(setRotateY(int)));
    connect(sliderZ, SIGNAL(valueChanged(int)),this,SLOT(setRotateZ(int)));
    connect(sliderH, SIGNAL(valueChanged(int)),this,SLOT(setRotateH(int)));

    connect(sliderAngle, SIGNAL(valueChanged(int)),this,SLOT(setAngle(int)));
    connect(sliderRatio, SIGNAL(valueChanged(int)),this,SLOT(setRatio(int)));
    connect(sliderNearPlane, SIGNAL(valueChanged(int)),this,SLOT(setNearPlane(int)));
    connect(sliderNarPlane, SIGNAL(valueChanged(int)),this,SLOT(setNarPlane(int)));

}

void Grid3dDlg::setRotateX(int val)
{
    w->rotateX = -val;
    w->update();
}

void Grid3dDlg::setRotateY(int val)
{
    w->rotateY = val;
    w->update();
}

void Grid3dDlg::setRotateZ(int val)
{
    w->rotateZ = val;
    w->update();
}

void Grid3dDlg::setRotateH(int val)
{
    w->h = val;
    w->update();
}

// ///////////////
void Grid3dDlg::setAngle(int val)
{
    w->verticalAngle = val;
    w->update();
}

void Grid3dDlg::setRatio(int val)
{
    w->aspectRatio = val/10.0;
     qDebug()<<"aspectRatio:"<<w->aspectRatio;
    w->update();
}

void Grid3dDlg::setNearPlane(int val)
{
    w->nearPlane = val/1000.0;
//    qDebug()<<"near:"<<val;
    w->update();
}

void Grid3dDlg::setNarPlane(int val)
{
//    qDebug()<<"far:"<<val;
    w->farPlane = val;
    w->update();
}
