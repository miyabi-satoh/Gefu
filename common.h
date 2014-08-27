#ifndef COMMON_H
#define COMMON_H

#include <QMap>
typedef QMap<QString, QString> StringMap;

class QObject;
class QKeyEvent;
class MainWindow;
extern MainWindow* getMainWnd();
extern QString FilesizeToString(quint64 size);
extern QString ViewerIgnoreExt();
extern QString KeyEventToSequence(const QKeyEvent *event);
extern bool ProcessShortcut(const QString &ksq, const QObject *object);

#define slash   QString("/")
#define QQ(x)   ("\"" + QString(x) + "\"")

#define IniKey_ShowHidden               "Common/ShowHidden"
#define IniKey_ShowSystem               "Common/ShowSystem"
#define IniKey_DefaultOnCopy            "Common/DefaultCopyMode"
#define IniKey_MoveAfterCreateFolder    "Common/MoveAfterFolderCreation"
#define IniKey_OpenAfterCreateFile      "Common/OpenAfterFileCreation"
#define IniKey_CheckUpdates             "Common/CheckUpdates"
#define IniKey_BootSizeSpec             "BootAndExit/SizeSpec"
#define IniKey_BootSizeRel              "BootAndExit/SizeRel"
#define IniKey_BootSizeAbs              "BootAndExit/SizeAbs"
#define IniKey_BootPosSpec              "BootAndExit/PosSpec"
#define IniKey_BootPosRel               "BootAndExit/PosRel"
#define IniKey_BootPosAbs               "BootAndExit/PosAbs"
#define IniKey_ResetOnBoot              "BootAndExit/Reset"
#define IniKey_BoxColorFg               "Appearance/BoxColorFg"
#define IniKey_BoxColorBg               "Appearance/BoxColorBg"
#define IniKey_BoxFont                  "Appearance/BoxFont"
#define IniKey_ViewFont                 "Appearance/ViewFont"
#define IniKey_ViewColorBgNormal        "Appearance/ViewColorBgNormal"
#define IniKey_ViewColorBgMark          "Appearance/ViewColorBgMark"
#define IniKey_ViewColorFgNormal        "Appearance/ViewColorFgNormal"
#define IniKey_ViewColorFgMark          "Appearance/ViewColorFgMark"
#define IniKey_ViewColorFgSystem        "Appearance/ViewColorFgSystem"
#define IniKey_ViewColorFgHidden        "Appearance/ViewColorFgHidden"
#define IniKey_ViewColorFgReadonly      "Appearance/ViewColorFgReadonly"
#define IniKey_ConfirmExit              "Confirm/Exit"
#define IniKey_ConfirmCopy              "Confirm/Copy"
#define IniKey_ConfirmMove              "Confirm/Move"
#define IniKey_ConfirmDelete            "Confirm/Delete"
#define IniKey_ConfirmRename            "Confirm/Rename"
#define IniKey_AutoCloseCopy            "AutoClose/Copy"
#define IniKey_AutoCloseMove            "AutoClose/Move"
#define IniKey_AutoCloseDelete          "AutoClose/Delete"
#define IniKey_AutoCloseRename          "AutoClose/Rename"
#define IniKey_EditorPath               "Editor/Path"
#define IniKey_EditorOption             "Editor/Option"
#define IniKey_TerminalPath             "Terminal/Path"
#define IniKey_TerminalOption           "Terminal/Option"
#define IniKey_ViewerFont               "Viewer/Font"
#define IniKey_ViewerColorBg            "Viewer/ColorBg"
#define IniKey_ViewerColorFg            "Viewer/ColorFg"
#define IniKey_ViewerInherit            "Viewer/Inherit"
#define IniKey_ViewerForceOpen          "Viewer/ForceOpen"
#define IniKey_ViewerIgnoreExt          "Viewer/IgnoreExt"
#define IniKey_WindowGeometry           "Window/Geometry"
#define iniKey_WindowState              "Window/State"
// 以下は[Left/Right]セクションのキー
#define IniKey_Dir                  "dir"
#define IniKey_SortBy               "SortBy"
#define IniKey_OrderBy              "OrderBy"
#define IniKey_PutDirs              "PutDirs"
#define IniKey_IgnoreCase           "IgnoreCase"

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
