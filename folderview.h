#ifndef FOLDERVIEW_H
#define FOLDERVIEW_H

#include "filetablemodel.h"
#include "history.h"

#include <QTableView>
class MainWindow;
class FolderPanel;

class FolderView : public QTableView
{
    Q_OBJECT
public:
    explicit FolderView(QWidget *parent = 0);

    QString side() const;

    // actions
    void initialize(MainWindow *mainWnd);

    void searchItem(const QString &text);
    void searchNext(const QString &text);
    void searchPrev(const QString &text);

    void setCheckStateAll(Qt::CheckState state);
    void setCheckStateAllFiles();
    void invertCheckState();
    void toggleCheckState(const QModelIndex &index);

    bool historyBack();
    bool historyForward();

    // getter
    QFileInfo currentItem() const;
    QString dir() const { return m_model.absolutePath(); }
    QFileInfoList checkedItems() const;
    QFileInfoList selectedItems() const;
    const History* history() const { return &m_history; }
    QStringList nameFilters() const { return m_model.nameFilters(); }
    FolderPanel *parentPanel() const;


    // setter
    void setPath(const QString &path, bool addHistory);
    void setFilter(QDir::Filters filter, bool enable);
    void setHistoryIndexAt(int index);
    void setNameFilters(const QStringList &list);
    void setSorting();

private:
    MainWindow *m_mainWnd;
    FileTableModel m_model;
    History m_history;
    QPoint m_dragStartPos;
    bool m_dragging;
    int m_saveRow;
    QString m_saveName;

signals:
    void currentChanged(const QFileInfo &info);
    void dataChanged();
    void dropAccepted(const QFileInfoList &list, QDropEvent *event);
    void itemFound();
    void itemNotFound();
    void retrieveStarted(const QString &path);
    void requestContextMenu(QContextMenuEvent *event);

public slots:
    void refresh();
    void preReload();
    void postReload();

    // QAbstractItemView interface
protected slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // FOLDERVIEW_H
