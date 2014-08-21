#include "common.h"
#include "filetablemodel.h"

#include <QDateTime>
#include <QDebug>
#include <QApplication>
#include <QSettings>
#include <QMenuBar>
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
        for (it = m_fileInfoList.begin(); it != m_fileInfoList.end(); ) {
            if (it->fileName() == ".." && m_dir.isRoot()) {
                it = m_fileInfoList.erase(it);
                continue;
            }
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
            it++;
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

#define Brush(x, y, z)  QBrush((x).value((y), (z)).value<QColor>());


void FileTableModel::updateAppearance()
{
    QSettings settings;
    QPalette palette(QApplication::palette("QTableView"));

    m_font = settings.value(IniKey_ViewFont, QApplication::font()).value<QFont>();
    m_NormalBrush = Brush(settings, IniKey_ViewColorBgNormal, palette.base());
    m_NormalTextBrush = Brush(settings, IniKey_ViewColorFgNormal, palette.text());
    m_MarkBrush = Brush(settings, IniKey_ViewColorBgMark, DefaultMarkBgColor);
    m_MarkTextBrush = Brush(settings, IniKey_ViewColorFgMark, DefaultMarkFgColor);
    m_SystemBrush = Brush(settings, IniKey_ViewColorFgSystem, DefaultSystemColor);
    m_HiddenBrush = Brush(settings, IniKey_ViewColorFgHidden, DefaultHiddenColor);
    m_ReadonlyBrush = Brush(settings, IniKey_ViewColorFgReadonly, DefaultReadonlyColor);

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
            if (dwFlags != DWORD(-1) && (dwFlags & FILE_ATTRIBUTE_SYSTEM) == FILE_ATTRIBUTE_SYSTEM))
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
    if (index.column() == 0 && m_fileInfoList[index.row()].fileName() != "..")
    {
        flags |= Qt::ItemIsUserCheckable;
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
