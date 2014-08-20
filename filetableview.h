#ifndef FILETABLEVIEW_H
#define FILETABLEVIEW_H

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
    void setRootPath(const QString &path);

private:
    QString m_side;

    QFileInfoList selectedItems() const;

signals:
    void indexChanged(const QString &text);

public slots:
    void setPath();
    void openUrl();
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

    void XXX();

private slots:
    void askOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod,
                      QString *alias, const QString &srcPath,
                      const QString &tgtPath);

    // QAbstractItemView interface
public slots:
    void setRootIndex(const QModelIndex &index);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);

    // QAbstractItemView interface
protected slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
};

#endif // FILETABLEVIEW_H
