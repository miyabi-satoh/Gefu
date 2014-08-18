#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>
class FolderPanel;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void setStatusText(const QString &str);

public slots:
    void onActionCommand();
    void onActionExec();
    void onActionOpen();
    void onActionQuit();
    void onActionSetting();

    void onMarkAll();
    void onMarkAllFiles();
    void onMarkAllOff();
    void onMarkInvert();
    void onMarkToggle();

    void onMoveCursorDown();
    void onMoveCursorUp();
    void onMoveCursorBegin();
    void onMoveCursorEnd();
    void onMoveBack();
    void onMoveForward();
    void onMoveHome();
    void onMoveJump();
    void onMoveOther();
    void onMoveParent();
    void onMoveRoot();

    void onViewFromOther();
    void onViewToOther();
    void onViewHidden();
    void onViewSwap();

    void onCmdMove();
    void onCmdCopy();
    void onCmdDelete();
    void onCmdNewFile();
    void onCmdNewFolder();
    void onCmdRename();

    void onHelpAbout();

private slots:
    void onAskOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod,
                        QString *alias, const QString srcPath, const QString tgtPath);

private:
    Ui::MainWindow *ui;

    FolderPanel* activePanel();
    FolderPanel* inactivePanel();
    QStringList selectedItems(FolderPanel *fp);
};


#endif // MAINWINDOW_H
