#include "common.h"
#include "filetablemodel.h"

#include <QDateTime>
#include <QDebug>
#ifdef Q_OS_WIN32
    #include <windows.h>
#endif

FileTableModel::FileTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_dir(),
    m_fileInfoList(),
    m_checkStates(),
    m_IconFactory(),
    m_fsWatcher(NULL)
{
}

bool FileTableModel::setPath(const QString &path)
{
    if (!QFileInfo::exists(path)) {
        qDebug() << "パスが存在しません";
        qDebug() << path;
        return false;
    }
    QFileInfo info(path);
    if (!info.isDir()) {
        qDebug() << "フォルダではありません";
        qDebug() << path;
        return false;
    }

    beginResetModel();

    m_dir.setPath(path);
    m_fileInfoList = m_dir.entryInfoList();
    m_checkStates.clear();

    if (m_fileInfoList.isEmpty()) {
        qDebug() << "ファイルリストを取得できません";
        qDebug() << path;
    }
#ifdef Q_OS_WIN32
    else if (!(filter() & QDir::System)){
        QFileInfoList::iterator it;
        for (it = m_fileInfoList.begin(); it != m_fileInfoList.end(); ) {
            DWORD dwFlags = ::GetFileAttributes(
                        it->absoluteFilePath().toStdWString().c_str());
            if (dwFlags != DWORD(-1) && it->fileName() != ".." &&
                ((dwFlags & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM))
            {
                qDebug() << it->fileName() << " is system file.";
                it = m_fileInfoList.erase(it);
            }
            else {
                it++;
            }
        }
    }
#endif
    m_checkStates.resize(m_fileInfoList.size());
    m_checkStates.fill(Qt::Unchecked);

    if (m_fsWatcher) {
        delete m_fsWatcher;
    }
    m_fsWatcher = new QFileSystemWatcher(this);
    m_fsWatcher->addPath(path);
    connect(m_fsWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(refresh()));

    endResetModel();

    emit rootChanged(m_dir.absolutePath());
    stateChanged();

    return !m_fileInfoList.isEmpty();
}

Qt::CheckState FileTableModel::checkState(const QModelIndex &index) const
{
    return m_checkStates[index.row()];
}

void FileTableModel::setCheckState(const QModelIndex &index, Qt::CheckState state)
{
    m_checkStates[index.row()] = state;
    stateChanged();
}

void FileTableModel::setCheckStateAll(Qt::CheckState state)
{
    beginResetModel();
    m_checkStates.fill(state);
    endResetModel();

    stateChanged();
}


bool FileTableModel::isDir(const QModelIndex &index) const
{
    return m_fileInfoList[index.row()].isDir();
}

const QString FileTableModel::absoluteFilePath(const QModelIndex &index) const
{
    return m_dir.absoluteFilePath(m_fileInfoList[index.row()].fileName());
}

QFileInfo FileTableModel::fileInfo(const QModelIndex &index) const
{
    return m_fileInfoList[index.row()];
}

void FileTableModel::stateChanged()
{
    int numFolder = 0;
    int numFile = 0;
    quint64 totalSize = 0;
    for (int n = 0; n < m_checkStates.size(); n++) {
        if (m_checkStates[n] == Qt::Checked) {
            if (m_fileInfoList[n].isDir()) {
                numFolder++;
            }
            else {
                numFile++;
                totalSize += m_fileInfoList[n].size();
            }
        }
    }

    emit stateChanged(numFolder, numFile, totalSize);
}

QFileInfoList FileTableModel::checkedItems() const
{
    QFileInfoList list;
    for (int n = 0; n < m_checkStates.size(); n++) {
        if (m_fileInfoList[n].fileName() != ".." &&
            m_checkStates[n] == Qt::Checked)
        {
            list.append(m_fileInfoList[n]);
        }
    }
    return list;
}

int FileTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_fileInfoList.size();
}

int FileTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 4;
}

QVariant FileTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QFileInfo &info = m_fileInfoList.at(index.row());
    int checked = m_checkStates.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0: // チェックボックス
            return QString("");
        case 1: // ファイル名
            return info.fileName();
        case 2: // サイズ
            if (info.isDir()) {
                return QString("<DIR>");
            }
            else {
                return FilesizeToString(info.size());
            }
            break;
        case 3:
            return info.lastModified().toString("yy/MM/dd hh:mm");
        }
        break;

    case Qt::DecorationRole:
        if (index.column() == 1) {
            if (info.fileName() == "..") {
                return QIcon(":/images/Up.png");
            }
            else {
                // infoを使うと、正しいアイコンが取れない場合がある…なぜ？
                return m_IconFactory.icon(QFileInfo(info.absoluteFilePath()));
            }
        }
        break;

    case Qt::TextAlignmentRole:
        switch (index.column()) {
        case 0:
        case 1:
            return Qt::AlignLeft + Qt::AlignVCenter;
        case 2:
        case 3:
            return Qt::AlignRight + Qt::AlignVCenter;
        }
        break;

    case Qt::CheckStateRole:
        if (index.column() == 0 && info.fileName() != "..") {
            return checked;
        }
        break;
    }

    return QVariant();
}

QVariant FileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case 0: return QString("");
            case 1: return tr("名前");
            case 2: return tr("サイズ");
            case 3: return tr("更新日時");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags FileTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == 0 && m_fileInfoList[index.row()].fileName() != "..")
    {
        flags |= Qt::ItemIsUserCheckable;
    }
    return flags;
}

bool FileTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    switch (role) {
    case Qt::CheckStateRole:
        if (index.column() == 0) {
            m_checkStates[index.row()] = static_cast<Qt::CheckState>(value.toInt());
            stateChanged();
            return true;
        }
        break;
    }

    return false;
}
