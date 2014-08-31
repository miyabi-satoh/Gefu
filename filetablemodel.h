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

    // action
    bool setPath(const QString &path);
    void updateAppearance(int darkness = false);

    // getter
    QDir::SortFlags sorting() const { return m_dir.sorting(); }
    QDir::Filters filter() const { return m_dir.filter(); }
    QStringList nameFilters() const { return m_dir.nameFilters(); }
    const QString absolutePath() const { return m_dir.absolutePath(); }
    QFileInfo fileInfo(const QModelIndex &index) const;
    QFont font() const { return m_font; }

    // setter
    void setSorting(QDir::SortFlags sort) { m_dir.setSorting(sort); }
    void setFilter(QDir::Filters filters) { m_dir.setFilter(filters); }
    void setNameFilters(const QStringList &nameFiltes) {
        m_dir.setNameFilters(nameFiltes);
    }

signals:
    void selectionChanged(int checkedFoldrs, int checkedFiles, quint64 totalSize);
    void preReload();
    void postReload();

public slots:
    void directoryChange(const QString &path);

private:
    QDir m_dir;
    QFileInfoList m_fileInfoList;
    QVector<Qt::CheckState> m_checkStates;
    QFileIconProvider m_IconFactory;
    QFileSystemWatcher m_fsWatcher;
    QFont m_font;
    QBrush m_NormalBrush;
    QBrush m_NormalTextBrush;
    QBrush m_MarkBrush;
    QBrush m_MarkTextBrush;
    QBrush m_SystemBrush;
    QBrush m_HiddenBrush;
    QBrush m_ReadonlyBrush;

    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
};

#endif // FILETABLEMODEL_H
