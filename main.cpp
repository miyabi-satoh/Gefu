#include "common.h"
#include "mainwindow.h"

#include <QApplication>
#include <QSettings>
#include <QDir>

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

    // 各オプションのデフォルト値を設定する
    //>>>>> 起動と終了
    if (settings.value(IniKey_ConfirmExit, "").toString().isEmpty())
        settings.setValue(IniKey_ConfirmExit, true);
    if (settings.value(IniKey_BootSizeSpec, "").toString().isEmpty()) {
        settings.setValue(IniKey_BootSizeSpec, "sizeLast");
        settings.setValue(IniKey_BootSizeAbs, QSize(800,600));
        settings.setValue(IniKey_BootSizeRel, QSize(50,50));
    }
    if (settings.value(IniKey_BootPosSpec, "").toString().isEmpty()) {
        settings.setValue(IniKey_BootPosSpec, "posLast");
        settings.setValue(IniKey_BootPosAbs, QPoint(0, 0));
        settings.setValue(IniKey_BootPosRel, QPoint(0, 0));
    }
    //>>>>> 色とフォント
    if (settings.value(IniKey_BoxFont, "").toString().isEmpty()) {
        settings.setValue(IniKey_BoxFont, a.font());
        settings.setValue(IniKey_BoxColorBg, QPalette().base().color());
        settings.setValue(IniKey_BoxColorFg, QPalette().text().color());
    }
    if (settings.value(IniKey_ViewFont, "").toString().isEmpty()) {
        settings.setValue(IniKey_ViewFont, a.font());
        settings.setValue(IniKey_ViewColorBgMark, QColor(0,192,0));
        settings.setValue(IniKey_ViewColorBgNormal, QPalette().base().color());
        settings.setValue(IniKey_ViewColorFgHidden, QColor(128,128,128));
        settings.setValue(IniKey_ViewColorFgMark, QColor(128,0,0));
        settings.setValue(IniKey_ViewColorFgNormal, QPalette().text().color());
        settings.setValue(IniKey_ViewColorFgReadonly, QColor(0,128,0));
        settings.setValue(IniKey_ViewColorFgSystem, QColor(128,0,128));
    }
    //>>>>>
    if (settings.value(IniKey_AutoCloseCopy, "").toString().isEmpty()) {
        settings.setValue(IniKey_AutoCloseCopy, false);
        settings.setValue(IniKey_AutoCloseDelete, false);
        settings.setValue(IniKey_AutoCloseMove, false);
        settings.setValue(IniKey_AutoCloseRename, false);
    }
    if (settings.value(IniKey_ConfirmCopy, "").toString().isEmpty()) {
        settings.setValue(IniKey_ConfirmCopy, true);
        settings.setValue(IniKey_ConfirmDelete, true);
        settings.setValue(IniKey_ConfirmMove, true);
        settings.setValue(IniKey_ConfirmRename, true);
    }
    if (settings.value(IniKey_DefaultOnCopy, "").toString().isEmpty())
        settings.setValue(IniKey_DefaultOnCopy, "owDefIfNew");
    if (settings.value(IniKey_MoveAfterCreateFolder, "").toString().isEmpty())
        settings.setValue(IniKey_MoveAfterCreateFolder, false);
    if (settings.value(IniKey_OpenAfterCreateFile, "").toString().isEmpty())
        settings.setValue(IniKey_OpenAfterCreateFile, false);
    //>>>>>
    if (settings.value(IniKey_EditorPath, "").toString().isEmpty()) {
#if defined(Q_OS_WIN)
        settings.setValue(IniKey_EditorPath, "notepad.exe");
#elif defined(Q_OS_MAC)
        settings.setValue(IniKey_EditorPath, "-t");
#else
        settings.setValue(IniKey_EditorPath, "gedit");
#endif
        settings.setValue(IniKey_EditorOption, QQ("$P"));
    }
    if (settings.value(IniKey_TerminalPath, "").toString().isEmpty()) {
#if defined(Q_OS_WIN)
        settings.setValue(IniKey_TerminalPath, "cmd.exe");
        settings.setValue(IniKey_TerminalOption, "/k cd " + QQ("$D"));
#elif defined(Q_OS_MAC)
        settings.setValue(IniKey_TerminalPath, "/Applications/Utilities/Terminal.app");
        settings.setValue(IniKey_TerminalOption, "-c cd " + QQ("$D"));
#else
        settings.setValue(IniKey_TerminalPath, "gnome-terminal");
        settings.setValue(IniKey_TerminalOption, "-c cd " + QQ("$D"));
#endif
    }
    //>>>>> 隠しファイルの表示
    if (settings.value(IniKey_ShowHidden, "").toString().isEmpty())
        settings.setValue(IniKey_ShowHidden, false);
    //>>>>> システムファイルの表示
    if (settings.value(IniKey_ShowSystem, "").toString().isEmpty())
        settings.setValue(IniKey_ShowSystem, false);
    //>>>>> 最後のフォルダとソート方法
    QString side = "Left/";
    if (settings.value(side + IniKey_Dir, "").toString().isEmpty()) {
        settings.setValue(side + IniKey_Dir, QDir::homePath());
        settings.setValue(side + IniKey_SortBy, SortByName);
        settings.setValue(side + IniKey_OrderBy, OrderByAsc);
        settings.setValue(side + IniKey_PutDirs, PutDirsFirst);
        settings.setValue(side + IniKey_IgnoreCase, true);
    }
    side = "Right/";
    if (settings.value(side + IniKey_Dir, "").toString().isEmpty()) {
        settings.setValue(side + IniKey_Dir, QDir::homePath());
        settings.setValue(side + IniKey_SortBy, SortByName);
        settings.setValue(side + IniKey_OrderBy, OrderByAsc);
        settings.setValue(side + IniKey_PutDirs, PutDirsFirst);
        settings.setValue(side + IniKey_IgnoreCase, true);
    }

    MainWindow w;
    w.show();

    return a.exec();
}
