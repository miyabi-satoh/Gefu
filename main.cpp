#include "common.h"
#include "mainwindow.h"

#include <QDebug>
#include <QApplication>
#include <QSettings>
#include <QDir>
#include <QAction>

bool ProcessShortcut(const QString &ksq, const QObject *object)
{
    qDebug() << "ProcessShortcut()";

    if (ksq.isEmpty()) {
        return false;
    }

    foreach (QObject *obj, object->children()) {
        QAction *action = qobject_cast<QAction*>(obj);
        if (!action || !action->isEnabled()) {
            continue;
        }

        foreach (const QKeySequence &k, action->shortcuts()) {
            if (ksq == k.toString()) {
                if (action->isCheckable()) {
                    action->setChecked(!action->isChecked());
                }
                else {
                    emit action->triggered();
                }
                qDebug() << "emit" << action->objectName();
                return true;
            }
        }
    }
    return false;
}

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

    // オラはやっちまっただぁ…
    QString strValue = settings.value("TerminalOption", "").toString();
    if (!strValue.isEmpty()) {
        settings.remove("TerminalOption");
        settings.setValue(IniKey_TerminalOption, strValue);
    }

    // 各オプションのデフォルト値を設定する
    //>>>>> 起動と終了
    if (settings.value(IniKey_ConfirmExit, "").toString().isEmpty()) {
        settings.setValue(IniKey_ConfirmExit, true);
        settings.setValue(IniKey_BootSizeSpec, "sizeRelative");
        settings.setValue(IniKey_BootSizeAbs, QSize(800,600));
        settings.setValue(IniKey_BootSizeRel, QSize(75,75));
        settings.setValue(IniKey_BootPosSpec, "posCenter");
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
    //>>>>> ファイル操作
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
    //>>>>> パス設定
    if (settings.value(IniKey_EditorPath, "").toString().isEmpty()) {
#if defined(Q_OS_WIN)
        settings.setValue(IniKey_EditorPath, "notepad.exe");
#elif defined(Q_OS_MAC)
        settings.setValue(IniKey_EditorPath, "/Applications/TextEdit.app");
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
    //>>>>> テキストビューア
    if (settings.value(IniKey_ViewerFont, "").toString().isEmpty()) {
        settings.setValue(IniKey_ViewerColorBg, QPalette().base().color());
        settings.setValue(IniKey_ViewerColorFg, QPalette().text().color());
        settings.setValue(IniKey_ViewerFont, a.font());
        settings.setValue(IniKey_ViewerForceOpen, false);
        settings.setValue(IniKey_ViewerInherit, true);
        settings.setValue(IniKey_ViewerIgnoreExt, ViewerIgnoreExt());
    }

    //>>>>> 隠しファイルの表示
    if (settings.value(IniKey_ShowHidden, "").toString().isEmpty())
        settings.setValue(IniKey_ShowHidden, false);
    //>>>>> システムファイルの表示
    if (settings.value(IniKey_ShowSystem, "").toString().isEmpty())
        settings.setValue(IniKey_ShowSystem, false);
    //>>>>> 最新版のチェック
    if (settings.value(IniKey_CheckUpdates, "").toString().isEmpty())
        settings.setValue(IniKey_CheckUpdates, true);
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

QString ViewerIgnoreExt()
{
    QStringList list;
    // 画像系
    list << "gif" << "jpg" << "jpeg" << "png" << "bmp" << "ico" << "ai";
    list << "psd" << "xcf" << "tif" << "tiff" << "wmf";
    // 音・動画系
    list << "wav" << "mp3" << "ogg" << "midi" << "mid" << "aif" << "aiff";
    list << "mov" << "mpg" << "mpeg" << "wma" << "wmv" << "asf" << "avi";
    list << "flac" << "mkv";
    // 実行ファイル系
    list << "exe" << "com" << "lib" << "dll" << "msi" << "scr" << "sys";
    list << "o" << "obj" << "ocx" << "a" << "so" << "app";
    // アーカイブ系
    list << "lzh" << "zip" << "cab" << "tar" << "rar" << "gz" << "tgz";
    list << "bz2" << "xz" << "jar" << "7z" << "dmg";
    // ドキュメント系
    list << "pdf" << "doc" << "docx" << "xls" << "xlsx" << "ppt" << "pptx";
    // フォント
    list << "ttf" << "ttc";

    list.sort();

    return list.join(",");
}
