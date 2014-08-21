#ifndef FILETABLEMODEL_H
#define FILETABLEMODEL_H

#include <QAbstractTableModel>
#include <QDir>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include <QBrush>
#include <QFont>

class FileTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit FileTableModel(QObject *parent = 0);

    bool setPath(const QString &path);

    QDir::Filters filter() const { return m_dir.filter(); }
    void setFilter(QDir::Filters filters) { m_dir.setFilter(filters); }

    QDir::SortFlags sorting() const { return m_dir.sorting(); }
    void setSorting(QDir::SortFlags sort) { m_dir.setSorting(sort); }

    Qt::CheckState checkState(const QModelIndex &index) const;
    void setCheckState(const QModelIndex &index, Qt::CheckState state);
    void setCheckStateAll(Qt::CheckState state);

    bool isDir(const QModelIndex &index) const;
    const QString absolutePath() const { return m_dir.absolutePath(); }
    const QString absoluteFilePath(const QModelIndex &index) const;
    QFileInfoList checkedItems() const;
    QFileInfo fileInfo(const QModelIndex &index) const;

    void updateAppearance();

signals:
    void rootChanged(const QString &root);
    void stateChanged(int checkedFoldrs, int checkedFiles, quint64 totalSize);
    void listUpdated();

public slots:
//    void refresh();

private:
    QDir m_dir;
    QFileInfoList m_fileInfoList;
    QVector<Qt::CheckState> m_checkStates;
    QFileIconProvider m_IconFactory;
    QFileSystemWatcher *m_fsWatcher;
    QFont m_font;
    QBrush m_NormalBrush;
    QBrush m_NormalTextBrush;
    QBrush m_MarkBrush;
    QBrush m_MarkTextBrush;
    QBrush m_SystemBrush;
    QBrush m_HiddenBrush;
    QBrush m_ReadonlyBrush;

    void stateChanged();

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
};

#endif // FILETABLEMODEL_H
