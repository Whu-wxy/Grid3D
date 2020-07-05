#include "grid3ddlg.h"
#include <QApplication>

#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Grid3dDlg w;
    w.show();

    return a.exec();
}
