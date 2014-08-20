#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>
class FolderPanel;
class FileTableView;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    FileTableView* otherSideView(const FileTableView *view) const;

signals:
    void showHiddenFiles(bool show);
    void showSystemFiles(bool show);

public slots:
    void onActionSetting();
    void onHelpAbout();

private slots:
    void toggleShowHiddenFiles();
    void toggleShowSystemFiles();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
