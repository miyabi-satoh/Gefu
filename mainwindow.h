#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

    void onCmdDelete();
    void onCmdNewFile();
    void onCmdNewFolder();
    void onCmdRename();

    void onHelpAbout();

private slots:

private:
    Ui::MainWindow *ui;

    FolderPanel* activePanel();
    FolderPanel* inactivePanel();
};

#endif // MAINWINDOW_H
