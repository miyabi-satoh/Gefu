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

    void setStatusText(const QString &str);
    FileTableView* otherSideView(const FileTableView *view) const;

public slots:
    void onActionSetting();
    void onViewFromOther();
    void onViewToOther();
    void onViewSwap();
    void onCmdMove();
    void onCmdCopy();
    void onHelpAbout();

private slots:
    void onAskOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod,
                        QString *alias, const QString srcPath, const QString tgtPath);

private:
    Ui::MainWindow *ui;

    FolderPanel* activePanel();
    FolderPanel* inactivePanel();
};


#endif // MAINWINDOW_H
