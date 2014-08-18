#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QMap>

typedef QMap<QString, QString> StringMap;

class MainWindow;
extern MainWindow* getMainWnd();

extern QString FilesizeToString(quint64 size);

#define VERSION_STRING  "0.01"
#define VERSION_VALUE   0.01

#endif // COMMON_H
