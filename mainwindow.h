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

    void onMarkToggle();
    void onMarkAllFiles();
    void onMarkAllOff();
    void onMarkInvert();

    void onMoveDown();
    void onMoveUp();
    void onMoveParent();
    void onMoveOther();

    void onViewFromOther();
    void onViewSwap();

private slots:
    void on_action_Quit_triggered();
    void on_action_Open_triggered();
    void on_action_Exec_triggered();
    void on_view_FromOther_triggered();
    void on_view_ToOther_triggered();
    void on_mark_Toggle_triggered();
    void on_mark_All_triggered();
    void on_mark_AllFiles_triggered();
    void on_mark_AllOff_triggered();
    void on_mark_Invert_triggered();
    void on_view_Swap_triggered();
    void on_move_Down_triggered();
    void on_move_Up_triggered();
    void on_move_Parent_triggered();
    void on_move_Root_triggered();

private:
    Ui::MainWindow *ui;

    FolderPanel* activePanel();
    FolderPanel* inactivePanel();

};

#endif // MAINWINDOW_H
