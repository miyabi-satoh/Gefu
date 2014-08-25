#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QModelIndex>
#include <QMainWindow>
class QNetworkReply;
class FolderView;
class SearchBox;
class OverWriteDialog;
class QLabel;

namespace Ui {
class MainWindow;
}

///////////////////////////////////////////////////////////////////////////////
/// \brief The MainWindow class
///
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    FolderView *otherSideView(const FolderView *view) const;

signals:
    void showHiddenFiles(bool show);
    void showSystemFiles(bool show);

public slots:
    void askOverWrite(QString *copyMethod, QString *alias,
                      const QString &srcPath, const QString &tgtPath);

    void focusChange(QWidget * old, QWidget * now);
    void keyPress(FolderView *view, QKeyEvent *event);
    void returnPressInSearchBox();

    void chooseFolder();
    void copyFilenameToClipboard();
    void copyFullpathTpClipboard();
    void copyItems();
    void createFile();
    void createFolder();
    void cursorDown();
    void cursorUp();
    void deleteItems();
    void executeCommand();
    void historyBack();
    void historyForward();
    void itemFound(FolderView *view);
    void itemNotFound(FolderView *view);
    void markAll();
    void markAllFiles();
    void markAllOff();
    void markInvert();
    void markToggle();
    void moveItems();
    void open(const QModelIndex &index = QModelIndex());
    void openEditor(const QString &path = QString());
    void openTerminal();
    void refresh();
    void renameItems();
    void retrieveFinish();
    void retrieveStart(const QString &path);
    void searchItem(const QString& text);
    void searchNext();
    void searchPrev();
    void setCursorToBegin();
    void setCursorToEnd();
    void setPathFromOther();
    void setPathToHome();
    void setPathToOther();
    void setPathToParent();
    void setPathToRoot();
    void shellExecute();
    void showFilterDialog();
    void showHistoryDialog();
    void showSortDialog();
    void swapView();
    void toggleSearchBox(bool checked);




    void openRequest(const QFileInfo &info);
    void showPreferenceDialog();
    void about();
    void toggleShowHiddenFiles(bool checked);
    void toggleShowSystemFiles(bool checked);
    void checkUpdate(bool silent = false);
    void checkUpdateFinished(QNetworkReply *reply, bool silent = false);
    void checkUpdateFinishedSilent(QNetworkReply *reply);
    void viewFinish(QWidget *sender);

private:
    Ui::MainWindow *ui;
    QWidget *m_focusedView;
    OverWriteDialog *m_overwriteDialog;

    // action
    void initActionConnections();
    static void replaceVars(QString &str, const QFileInfo info);
    bool startProcess(const QString &cmd, const QString &workDir, const QString &errMsg);
    void updateActions();
    void setEnabledAllActions(bool enable);
    void setNameFilters(FolderView *view, const QString& filters = QString());
    void setSorting(FolderView *view);

    // getter
    QLabel* filterLabel(const FolderView *view) const;
    FolderView* folderView() const;
    SearchBox* searchBox(FolderView *view) const;


    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
