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
    m_fsWatcher(this),
    m_font(),
    m_NormalBrush(),
    m_NormalTextBrush(),
    m_MarkBrush(),
    m_MarkTextBrush(),
    m_SystemBrush(),
    m_HiddenBrush(),
    m_ReadonlyBrush()
{
    // デフォルトフィルタを設定する
    setFilter(QDir::NoDot | QDir::AllDirs | QDir::Files);
}

bool FileTableModel::setPath(const QString &path)
{
    qDebug() << "FileTableModel::setPath" << path;

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
                    continue;
                }
            }
            it++;
        }
        // ソート方法によらず、".."は必ず先頭にする
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

    m_checkStates.resize(m_fileInfoList.size());
    m_checkStates.fill(Qt::Unchecked);

    m_fsWatcher.addPath(path);
    connect(&m_fsWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(directoryChange(QString)));

    endResetModel();

    emit dataChanged(QModelIndex(), QModelIndex());

    return !m_fileInfoList.isEmpty();
}

QFileInfo FileTableModel::fileInfo(const QModelIndex &index) const
{
    qDebug() << "FileTableModel::fileInfo()" << index;

    if (index.isValid()) {
        return m_fileInfoList[index.row()];
    }
    return QFileInfo();
}

void FileTableModel::updateAppearance(bool darker)
{
    qDebug() << "FileTableModel::updateAppearance();" << darker;

    QSettings settings;
    int darkness = 100;
    if (darker) {
        darkness += settings.value(IniKey_Darkness).toInt();
    }
    m_font = settings.value(IniKey_ViewFont).value<QFont>();

    m_NormalBrush = QBrush(settings.value(IniKey_ViewColorBgNormal).value<QColor>().darker(darkness));
    m_NormalTextBrush = QBrush(settings.value(IniKey_ViewColorFgNormal).value<QColor>().darker(darkness));
    m_MarkBrush = QBrush(settings.value(IniKey_ViewColorBgMark).value<QColor>().darker(darkness));
    m_MarkTextBrush = QBrush(settings.value(IniKey_ViewColorFgMark).value<QColor>().darker(darkness));
    m_SystemBrush = QBrush(settings.value(IniKey_ViewColorFgSystem).value<QColor>().darker(darkness));
    m_HiddenBrush = QBrush(settings.value(IniKey_ViewColorFgHidden).value<QColor>().darker(darkness));
    m_ReadonlyBrush = QBrush(settings.value(IniKey_ViewColorFgReadonly).value<QColor>().darker(darkness));
    m_ReadonlyBrush = QBrush(settings.value(IniKey_ViewColorFgReadonly).value<QColor>().darker(darkness));
}

void FileTableModel::directoryChange(const QString &path)
{
    qDebug() << "FileTableModel::directoryChange" << path;

    m_fsWatcher.removePath(m_dir.absolutePath());
    setPath(path);
}

int FileTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_fileInfoList.size();
}

int FileTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 5;
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
            if (info.isDir()) {
                return info.fileName();
            }
            if (info.fileName().left(1) == ".") {   // "."で始まるファイル
                int pos = info.fileName().lastIndexOf(".");
                if (pos == 0) {
                    return info.fileName();
                }
            }
            return info.completeBaseName();

        case 2: // 拡張子
            if (info.isDir()) {
                return QString();
            }
            if (info.fileName().left(1) == ".") {   // "."で始まるファイル
                int pos = info.fileName().lastIndexOf(".");
                if (pos == 0) {
                    return QString();
                }
            }
            return info.suffix();

        case 3: // サイズ
            if (info.isDir()) {
                return QString("<DIR>");
            }
            else {
                return FilesizeToString(info.size());
            }
            break;

        case 4: // 更新日時
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
//                return m_IconFactory.icon(QFileInfo(info.absoluteFilePath()));
                return m_IconFactory.icon(QFileInfo(info));
            }
        }
        break;

    case Qt::FontRole:
        return m_font;

    case Qt::TextAlignmentRole:
        switch (index.column()) {
        case 3:
        case 4:
            return Qt::AlignRight + Qt::AlignVCenter;
        default:
            return Qt::AlignLeft + Qt::AlignVCenter;
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
            case 2: return tr("拡張子");
            case 3: return tr("サイズ");
            case 4: return tr("更新日時");
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags FileTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (!index.isValid()) {
        return flags | Qt::ItemIsDropEnabled;
    }

    if (m_fileInfoList[index.row()].fileName() != "..") {
        if (index.column() == 0) {
            flags |= Qt::ItemIsUserCheckable;
        }
        flags |= Qt::ItemIsDropEnabled;
    }
    return flags;
}

bool FileTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    qDebug() << "FileTableModel::setData()" << index;

    if (!index.isValid()) {
        return false;
    }

    switch (role) {
    case Qt::CheckStateRole:
        if (index.column() == 0) {
            m_checkStates[index.row()] = static_cast<Qt::CheckState>(value.toInt());
            emit dataChanged(index, this->index(index.row(), 4));
            return true;
        }
        break;
    }

    return false;
}

Qt::DropActions FileTableModel::supportedDropActions() const
{
    qDebug() << "FileTableModel::supportedDropActions()";

    return Qt::CopyAction;
}

QStringList FileTableModel::mimeTypes() const
{
    qDebug() << "FileTableModel::mimeTypes()";

    QStringList types;

    types << "text/uri-list";

    return types;
}
