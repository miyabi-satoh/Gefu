#include "common.h"
#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("miyabi");
    a.setOrganizationDomain("rakusaba.jp");
    a.setApplicationName("Gefu");
#if defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACX)
    a.setWindowIcon(QIcon(":/images/Gefu.icns"));
#else
    a.setWindowIcon(QIcon(":/images/Gefu.png"));
#endif
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings settings;
    if (settings.value(IniKey_ResetOnBoot, false).toBool()) {
        settings.clear();
    }

    MainWindow w;
    w.show();

    return a.exec();
}
