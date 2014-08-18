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

#define slash   QString("/")

#define IniKey_ShowHidden       "Common/ShowHidden"
#define IniKey_ShowSystem       "Common/ShowSystem"
#define IniSec_Left             "Left"
#define IniSec_Right            "Right"
#define IniKey_Dir              "dir"
#define IniKey_SortBy           "SortBy"
#define IniKey_OrderBy          "OrderBy"
#define IniKey_PutDirs          "PutDirs"
#define IniKey_IgnoreCase       "IgnoreCase"
#define IniKey_LeftDir          IniSec_Left"/"IniKey_Dir
#define IniKey_LeftSortBy       IniSec_Left"/"IniKey_SortBy
#define IniKey_LeftOrderBy      IniSec_Left"/"IniKey_OrderBy
#define IniKey_LeftPutDirs    IniSec_Left"/"IniKey_PutDirs
#define IniKey_LeftIgnoreCase   IniSec_Left"/"IniKey_IgnoreCase
#define IniKey_RightDir         IniSec_Right"/"IniKey_Dir
#define IniKey_RightSortBy      IniSec_Right"/"IniKey_SortBy
#define IniKey_RightOrderBy     IniSec_Right"/"IniKey_OrderBy
#define IniKey_RightPutDirs   IniSec_Right"/"IniKey_PutDirs
#define IniKey_RightIgnoreCase  IniSec_Right"/"IniKey_IgnoreCase

#define SortByName  QDir::Name
#define SortByDate  QDir::Time
#define SortBySize  QDir::Size
#define SortByType  QDir::Type

#define OrderByAsc  0
#define OrderByDesc QDir::Reversed

#define PutDirsFirst    QDir::DirsFirst
#define PutDirsLast     QDir::DirsLast
#define PutDirsDefault  0


#endif // COMMON_H
