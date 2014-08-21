#ifndef COMMON_H
#define COMMON_H

#include <QMap>
typedef QMap<QString, QString> StringMap;

class MainWindow;
extern MainWindow* getMainWnd();
extern QString FilesizeToString(quint64 size);

#define VERSION_VALUE   0.03

#define slash   QString("/")
#define QQ(x)   ("\"" + (x) + "\"")

#define IniKey_ShowHidden       "Common/ShowHidden"
#define IniKey_ShowSystem       "Common/ShowSystem"
#define IniKey_ConfirmExit      "BootAndExit/ConfirmExit"
#define IniKey_BootSizeSpec     "BootAndExit/SizeSpec"
#define IniKey_BootSizeRel      "BootAndExit/SizeRel"
#define IniKey_BootSizeAbs      "BootAndExit/SizeAbs"
#define IniKey_BootPosSpec      "BootAndExit/PosSpec"
#define IniKey_BootPosRel       "BootAndExit/PosRel"
#define IniKey_BootPosAbs       "BootAndExit/PosAbs"
#define IniKey_ResetOnBoot      "BootAndExit/Reset"
#define IniKey_BoxColorFg       "Appearance/BoxColorFg"
#define IniKey_BoxColorBg       "Appearance/BoxColorBg"
#define IniKey_BoxFont          "Appearance/BoxFont"
#define IniKey_ViewFont         "Appearance/ViewFont"
#define IniKey_ViewColorBgNormal    "Appearance/ViewColorBgNormal"
#define IniKey_ViewColorBgMark      "Appearance/ViewColorBgMark"
#define IniKey_ViewColorFgNormal    "Appearance/ViewColorFgNormal"
#define IniKey_ViewColorFgMark      "Appearance/ViewColorFgMark"
#define IniKey_ViewColorFgSystem    "Appearance/ViewColorFgSystem"
#define IniKey_ViewColorFgHidden    "Appearance/ViewColorFgHidden"
#define IniKey_ViewColorFgReadonly  "Appearance/ViewColorFgReadonly"
#define IniKey_WindowGeometry   "Window/Geometry"
#define iniKey_WindowState      "Window/State"
#define IniSec_Left             "Left"
#define IniSec_Right            "Right"
#define IniKey_Dir              "dir"
#define IniKey_SortBy           "SortBy"
#define IniKey_OrderBy          "OrderBy"
#define IniKey_PutDirs          "PutDirs"
#define IniKey_IgnoreCase       "IgnoreCase"

#define SortByName  QDir::Name
#define SortByDate  QDir::Time
#define SortBySize  QDir::Size
#define SortByType  QDir::Type

#define OrderByAsc  0
#define OrderByDesc QDir::Reversed

#define PutDirsFirst    QDir::DirsFirst
#define PutDirsLast     QDir::DirsLast
#define PutDirsDefault  0

#define DefaultMarkBgColor      QColor(0, 192, 0)
#define DefaultMarkFgColor      QColor(128, 0, 0)
#define DefaultSystemColor      QColor(128, 0, 128)
#define DefaultHiddenColor      QColor(128, 128, 128)
#define DefaultReadonlyColor    QColor(0, 128, 0)

#endif // COMMON_H
