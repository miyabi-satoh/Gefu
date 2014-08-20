#ifndef FILETABLEVIEW_H
#define FILETABLEVIEW_H

#include <QFileInfoList>
#include <QTableView>

class FileTableView : public QTableView
{
    Q_OBJECT
public:
    explicit FileTableView(QWidget *parent = 0);

    QString side() const;
    void setSide(const QString &side);

private:
    QString m_side;

    QFileInfoList selectedItems() const;

signals:

public slots:
    void setPath();
    void openUrl();
    void kickProcess();

    void toggleChecked();
    void checkAllItems();
    void checkAllFiles();
    void uncheckAllItems();
    void invertAllChecked();

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

    void cmdDelete();
    void cmdRename();
    void newFile();
    void newFolder();

    void XXX();

    // QAbstractItemView interface
public slots:
    void setRootIndex(const QModelIndex &index);

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // FILETABLEVIEW_H
