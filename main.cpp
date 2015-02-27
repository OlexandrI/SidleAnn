#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("FrontPictures");
    QCoreApplication::setOrganizationDomain("frontpictures.com");
    QCoreApplication::setApplicationName("SidleAnn");
    MainWindow w;
    w.show();

    return a.exec();
}
