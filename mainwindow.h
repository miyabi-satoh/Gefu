#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFileInfo>
#include <QModelIndex>
#include <QMainWindow>
class QNetworkReply;
class OverWriteDialog;
class QLabel;
class FolderView;
class SearchBox;
class AnyView;

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

signals:
    void showHiddenFiles(bool show);
    void showSystemFiles(bool show);

public slots:
    void askOverWrite(QString *copyMethod, QString *alias,
                      const QString &srcPath, const QString &tgtPath);

    void currentChange(const QFileInfo &info);
    void dropAccept(const QFileInfoList &list);
    void focusChange(QWidget * old, QWidget * now);
    void leftKeyPress();
    void rightKeyPress();
    void showFileInfo(const QString &str);

    void about();
    void checkUpdate(bool silent = false);
    void checkUpdateFinished(QNetworkReply *reply, bool silent = false);
    void checkUpdateFinishedSilent(QNetworkReply *reply);
    void chooseFolder();
    void copyFilenameToClipboard();
    void copyFullpathTpClipboard();
    void copyItems();
    void createFile();
    void createFolder();
    void cursorDown();
    void cursorUp();
    void cursorDownOther();
    void cursorUpOther();
    void deleteItems();
    void editBookmark();
    void executeCommand();
    void historyBack();
    void historyForward();
    void markAll();
    void markAllFiles();
    void markAllOff();
    void markInvert();
    void markToggle();
    void moveItems();
    void open(const QModelIndex &index = QModelIndex());
    void openEditor(const QString &path = QString());
    void openTerminal();
    void openBookmark();
    void refresh();
    void renameItems();
    void searchNext();
    void searchPrev();
    void setCursorToBegin();
    void setCursorToEnd();
    void setCursorToBeginOther();
    void setCursorToEndOther();
    void setFontSizeDown();
    void setFontSizeUp();
    void setPathFromOther();
    void setPathToHome();
    void setPathToOther();
    void setPathToParent();
    void setPathToRoot();
    void shellExecute();
    void showBookmarkDialog();
    void showFilterDialog();
    void showHistoryDialog();
    void showPreferenceDialog();
    void showSortDialog();
    void swapView();
    void switchHalfMode(bool checked);
    void toggleSearchBox(bool checked);
    void toggleShowHiddenFiles(bool checked);
    void toggleShowSystemFiles(bool checked);

    void showContextMenu(QContextMenuEvent *event);

    void viewFinish();

private:
    enum Mode {
        ModeBasic = 0x01,
        ModeSearch = 0x02,
        ModeFullView = 0x04,
        ModeHalfView = 0x08,
    };
    typedef QFlags<Mode> ModeFlags;

    Ui::MainWindow *ui;
    QWidget *m_focusedView;
    OverWriteDialog *m_overwriteDialog;
    ModeFlags m_viewMode;

    // action
    void initActionConnections();
    static void replaceVars(QString &str, const QFileInfo info);
    bool startProcess(const QString &cmd, const QString &workDir, const QString &errMsg);
    void updateActions();
    void setEnabledAllActions(bool enable);
    void showNameFilters(FolderView *view);
    void copyItems(const QFileInfoList &list, const QString &tgtDir);
    void changeFontSize(int diff);
    void initBookmark();
    void sendEventOther(QEvent *event);

    // getter
    FolderView* otherSideFolderView(const FolderView *view) const;

    // setter
    void setViewMode(ModeFlags flags);

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
