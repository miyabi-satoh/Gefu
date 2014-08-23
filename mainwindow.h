#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QMainWindow>
class FolderPanel;
class FileTableView;
class QNetworkReply;

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

private slots:
    void openRequest(const QFileInfo &info);
    void showPreferenceDialog();
    void about();
    void toggleShowHiddenFiles();
    void toggleShowSystemFiles();
    void checkUpdate(bool silent = false);
    void checkUpdateFinished(QNetworkReply *reply);
    void checkUpdateFinished(QNetworkReply *reply, bool silent);
    void checkUpdateFinishedSilent(QNetworkReply *reply);
    void viewFinish(QWidget *sender);

private:
    Ui::MainWindow *ui;
    QWidget *m_focusedView;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // MAINWINDOW_H
