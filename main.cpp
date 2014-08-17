#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("miyabi");
    a.setOrganizationDomain("rakusaba.jp");
    a.setApplicationName("Gefu");
    a.setWindowIcon(QIcon(":/images/Gefu.png"));

    QSettings::setDefaultFormat(QSettings::IniFormat);

    MainWindow w;
    w.show();

    return a.exec();
}
