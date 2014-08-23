#include "common.h"
#include "filetablemodel.h"

#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QSettings>
#include <QPalette>
#ifdef Q_OS_WIN32
    #include <windows.h>
#endif

FileTableModel::FileTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_dir(),
    m_fileInfoList(),
    m_checkStates(),
    m_IconFactory(),
    m_fsWatcher(NULL),
    m_font(),
    m_NormalBrush(),
    m_NormalTextBrush(),
    m_MarkBrush(),
    m_MarkTextBrush(),
    m_SystemBrush(),
    m_HiddenBrush(),
    m_ReadonlyBrush()
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
    else {
        QFileInfoList::iterator it;
        bool removeDotDot = false;
        for (it = m_fileInfoList.begin(); it != m_fileInfoList.end(); ) {
#ifdef Q_OS_WIN32
            if (!(filter() & QDir::System)){
                DWORD dwFlags = ::GetFileAttributes(
                            it->absoluteFilePath().toStdWString().c_str());
                if (dwFlags != DWORD(-1) && it->fileName() != ".." &&
                    ((dwFlags & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM))
                {
                    qDebug() << it->fileName() << " is system file.";
                    it = m_fileInfoList.erase(it);
                    continue;
                }
            }
#endif
            if (it->fileName() == "..") {
                if (m_dir.isRoot()) {
                    qDebug() << m_dir.absolutePath() << " is root.";
                    it = m_fileInfoList.erase(it);
                    removeDotDot = true;
                    continue;
                }
            }
            it++;
        }
        // ソート方法によらず、".."は必ず先頭にする
        if (!removeDotDot) {
            QFileInfoList::iterator itRoot = m_fileInfoList.end();
            for (it = m_fileInfoList.begin(); it != m_fileInfoList.end(); it++) {
                if (it->fileName() == "..") {
                    itRoot = it;
                    break;
                }
            }
            if (itRoot != m_fileInfoList.end()) {
                QFileInfo info(*itRoot);
                m_fileInfoList.erase(itRoot);
                m_fileInfoList.push_front(info);
            }
        }
    }

    m_checkStates.resize(m_fileInfoList.size());
    m_checkStates.fill(Qt::Unchecked);

    if (m_fsWatcher) {
        delete m_fsWatcher;
    }
    m_fsWatcher = new QFileSystemWatcher(this);
    m_fsWatcher->addPath(path);
    connect(m_fsWatcher, SIGNAL(directoryChanged(QString)),
            this, SIGNAL(listUpdated()));

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
    beginResetModel();
    m_checkStates[index.row()] = state;
    if (state == Qt::Checked && index.row() == 0 &&
        m_fileInfoList[0].fileName() == "..")
    {
        m_checkStates[0] = Qt::Unchecked;
    }
    endResetModel();;
    emit dataChanged(index, this->index(index.row(), 3));

    stateChanged();
}

void FileTableModel::setCheckStateAll(Qt::CheckState state)
{
    beginResetModel();
    m_checkStates.fill(state);
    if (state == Qt::Checked && m_fileInfoList.size() > 1 &&
        m_fileInfoList[0].fileName() == "..")
    {
        m_checkStates[0] = Qt::Unchecked;
    }
    endResetModel();

    stateChanged();
}

#if 0
bool FileTableModel::isDir(const QModelIndex &index) const
{
    if (!index.isValid()) {
        qDebug() << "isDir() : index is invalid.";
        return QString();
    }
    return m_fileInfoList[index.row()].isDir();
}

const QString FileTableModel::absoluteFilePath(const QModelIndex &index) const
{
    if (!index.isValid()) {
        qDebug() << "absoluteFilePath() : index is invalid.";
        return QString();
    }
//    return m_dir.absoluteFilePath(m_fileInfoList[index.row()].fileName());
    return m_fileInfoList[index.row()].absoluteFilePath();
}

const QString FileTableModel::fileName(const QModelIndex &index) const
{
    if (!index.isValid()) {
        qDebug() << "fileName() : index is invalid.";
        return QString();
    }
    return m_fileInfoList[index.row()].fileName();
}
#endif

QFileInfo FileTableModel::fileInfo(const QModelIndex &index) const
{
    if (!index.isValid()) {
        qDebug() << "fileInfo() : index is invalid.";
        return QFileInfo();
    }
    return m_fileInfoList[index.row()];
}

void FileTableModel::updateAppearance()
{
    QSettings settings;
    QPalette palette(QApplication::palette("QTableView"));

    m_font = settings.value(IniKey_ViewFont).value<QFont>();
    m_NormalBrush = QBrush(settings.value(IniKey_ViewColorBgNormal).value<QColor>());
    m_NormalTextBrush = QBrush(settings.value(IniKey_ViewColorFgNormal).value<QColor>());
    m_MarkBrush = QBrush(settings.value(IniKey_ViewColorBgMark).value<QColor>());
    m_MarkTextBrush = QBrush(settings.value(IniKey_ViewColorFgMark).value<QColor>());
    m_SystemBrush = QBrush(settings.value(IniKey_ViewColorFgSystem).value<QColor>());
    m_HiddenBrush = QBrush(settings.value(IniKey_ViewColorFgHidden).value<QColor>());
    m_ReadonlyBrush = QBrush(settings.value(IniKey_ViewColorFgReadonly).value<QColor>());
    m_ReadonlyBrush = QBrush(settings.value(IniKey_ViewColorFgReadonly).value<QColor>());

    beginResetModel();
    endResetModel();
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

    case Qt::FontRole:
        return m_font;

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

    case Qt::BackgroundRole:
        if (checked) {
            return m_MarkBrush;
        }
        return m_NormalBrush;

    case Qt::ForegroundRole:
        if (checked) {
            return m_MarkTextBrush;
        }
#ifdef Q_OS_WIN32
        {
            DWORD dwFlags = ::GetFileAttributes(
                        info.absoluteFilePath().toStdWString().c_str());
            if (dwFlags != DWORD(-1) && (dwFlags & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM)
            {
                return m_SystemBrush;
            }
        }
#endif
        if (info.isHidden() && info.fileName() != "..") {
            return m_HiddenBrush;
        }
        if (!info.isWritable()) {
            return m_ReadonlyBrush;
        }
        return m_NormalTextBrush;

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
    if (!index.isValid()) {
        flags |= Qt::ItemIsDropEnabled;
    }
    else if (m_fileInfoList[index.row()].fileName() != "..") {
        if (index.column() == 0) {
            flags |= Qt::ItemIsUserCheckable;
        }
        flags |= Qt::ItemIsDropEnabled;
    }
    return flags;
}

bool FileTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid()) {
        return false;
    }

    switch (role) {
    case Qt::CheckStateRole:
        if (index.column() == 0) {
            m_checkStates[index.row()] = static_cast<Qt::CheckState>(value.toInt());
            emit dataChanged(index, this->index(index.row(), 3));
            stateChanged();
            return true;
        }
        break;
    }

    return false;
}

Qt::DropActions FileTableModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList FileTableModel::mimeTypes() const
{
    QStringList types;

    types << "text/uri-list";

    return types;
}
