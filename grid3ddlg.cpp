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
    sliderX->setRange(-180, 180);
    sliderX->setValue(0);
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
    sliderH->setRange(0, 300);
    sliderH->setValue(0);
    sliderH->setSliderPosition(144);
    // //////////////////////////
    sliderAngle->setTickInterval(1);
    sliderAngle->setRange(20, 180);
    sliderAngle->setValue(0);
    sliderAngle->setSliderPosition(30);

    sliderRatio->setTickInterval(0.1);
    sliderRatio->setRange(0, 20);
    sliderRatio->setValue(0);
    sliderRatio->setSliderPosition(0);

    sliderNearPlane->setTickInterval(1);
    sliderNearPlane->setRange(0, 200);
    sliderNearPlane->setValue(0);
    sliderNearPlane->setSliderPosition(288-100*1.732);

    sliderNarPlane->setTickInterval(1);
    sliderNarPlane->setRange(0, 500);
    sliderNarPlane->setValue(0);
    sliderNarPlane->setSliderPosition(288);

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
    w->rotateX = val;
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
    w->aspectRatio = val;
    w->update();
}

void Grid3dDlg::setNearPlane(int val)
{
    w->nearPlane = val;
    qDebug()<<"near:"<<val;
    w->update();
}

void Grid3dDlg::setNarPlane(int val)
{
    qDebug()<<"far:"<<val;
    w->farPlane = val;
    w->update();
}
