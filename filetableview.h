#ifndef FILETABLEVIEW_H
#define FILETABLEVIEW_H

#include "history.h"

#include <QFileInfoList>
#include <QTableView>

class FileTableView : public QTableView
{
    Q_OBJECT
public:
    explicit FileTableView(QWidget *parent = 0);
    ~FileTableView();

    QString side() const;
    void setSide(const QString &side);

    History* history() { return &m_history; }

    void setRootPath(const QString &path, bool addHistory);

private:
    QString m_side;
    History m_history;
    QPoint m_dragStartPos;
    bool m_dragging;

    QFileInfoList selectedItems() const;
    void updateMenu();

signals:
    void indexChanged(const QString &text);
    void filterChanged();
    void openRequested(const QFileInfo &info);

private slots:
    void openItem();
    void openUrl(const QModelIndex &index = QModelIndex());
    void openEditor(const QString &path = QString());
    void openTerminal(const QString &path = QString());
    void kickProcess();

    void toggleChecked();
    void checkAllItems();
    void checkAllFiles();
    void uncheckAllItems();
    void invertAllChecked();

    void setPathFromOther();
    void setPathToOther();
    void swapPath();
    void showHiddenFiles(bool show);
    void showSystemFiles(bool show);
    void setSort();
    void refresh();
    void setFilter();

    void back();
    void forward();
    void showHidtory();
    void jumpToHome();
    void jumpToParent();
    void jumpToRoot();
    void jumpTo();
    void cursorDown();
    void cursorUp();
    void cursorToBegin();
    void cursorToEnd();

    void cmdCopy();
    void cmdMove();
    void cmdDelete();
    void cmdRename();
    void newFile();
    void newFolder();
    void copyFilenameToClipboard();
    void copyFullpathToClipboard();

    void XXX();

private slots:
    void askOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod,
                      QString *alias, const QString &srcPath,
                      const QString &tgtPath);
    void onDoubleClick(const QModelIndex &index);
    void contextMenuRequested(const QPoint & pos);

    // QAbstractItemView interface
public slots:
    void setRootIndex(const QModelIndex &index);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
    void focusInEvent(QFocusEvent *event);
    void dropEvent(QDropEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    // QAbstractItemView interface
protected slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // FILETABLEVIEW_H
