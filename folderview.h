#ifndef FOLDERVIEW_H
#define FOLDERVIEW_H

#include "filetablemodel.h"
#include "history.h"

#include <QTableView>

class FolderView : public QTableView
{
    Q_OBJECT
public:
    explicit FolderView(QWidget *parent = 0);

    QString side() const;

    // actions
    void initialize();
    void updateAppearance();
    void refresh();

    void searchItem(const QString &text);
    void searchNext(const QString &text);
    void searchPrev(const QString &text);

    void setCheckStateAll(bool checked);
    void setCheckStateAllFiles();
    void invertCheckState();
    void toggleCheckState(const QModelIndex &index);

    bool historyBack();
    bool historyForward();


    // getter
    QFileInfo currentItem() const;
    QString dir() const { return m_model.absolutePath(); }
    QFileInfoList selectedItems() const;
    const History* history() const { return &m_history; }
    QStringList nameFilters() const { return m_model.nameFilters(); }

    // setter
    void setPath(const QString &path, bool addHistory);
    void setFilter(QDir::Filters filter, bool enable);
    void setHistoryIndexAt(int index);
    void setNameFilters(const QStringList &list);
    void setSorting(QDir::SortFlags flags);

private:
    FileTableModel m_model;
    History m_history;


signals:
    void itemFound(FolderView *view);
    void itemNotFound(FolderView *view);
    void retrieveStarted(const QString &path);
    void retrieveFinished();
    void keyPressed(FolderView *view, QKeyEvent *event);

public slots:


    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);

};

#endif // FOLDERVIEW_H
