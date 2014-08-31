#include "common.h"
#include "mainwindow.h"
#include "folderpanel.h"
#include "folderview.h"

#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QSettings>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
#include <QMenu>
#include <QStatusBar>

QString FilesizeToString(quint64 size)
{
    if (size >= 1024 * 1024 * 1024) {
        return QString("%1GB").arg(int(10 * size / (1024 * 1024 * 1024)) / 10);
    }
    if (size >= 1024 * 1024) {
        return QString("%1MB").arg(int(10 * size / (1024 * 1024)) / 10);
    }
    if (size >= 1024) {
        return QString("%1KB").arg(int(10 * size / (1024)) / 10);
    }
    return QString("%1B").arg(size);
}

FolderView::FolderView(QWidget *parent) :
    QTableView(parent),
    m_mainWnd(NULL),
    m_model(),
    m_dragStartPos(),
    m_dragging(false)
{
    setModel(&m_model);
    connect(&m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));
    connect(&m_model, SIGNAL(preReload()), this, SLOT(preReload()));
    connect(&m_model, SIGNAL(postReload()), this, SLOT(postReload()));

    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(true);

    setContextMenuPolicy(Qt::DefaultContextMenu);
}

QString FolderView::side() const
{
    if (this->objectName() == "folderView1") {
        return QString("Left");
    }
    else {
        Q_ASSERT(this->objectName() == "folderView2");
        return QString("Right");
    }
}

void FolderView::initialize(MainWindow *mainWnd)
{
    qDebug() << side() << "initialize";

    m_mainWnd = mainWnd;

    setColumnWidth(0, 30);

    // ソート順を初期化する
    setSorting();

    // ネームフィルタを初期化する
    setNameFilters(QStringList() << "*");

    // 前回終了時のパスを開く
    QSettings settings;
    setPath(settings.value(side() + slash + IniKey_Dir).toString(), true);
}

void FolderView::refresh()
{
    qDebug() << side() << "refresh";

    preReload();

    setPath(m_model.absolutePath(), false);

    postReload();
}

void FolderView::preReload()
{
    // 現在行と名前を保存しておく
    m_saveRow = currentIndex().row();
    m_saveName = currentItem().fileName();
}

void FolderView::postReload()
{
    // 保存した名前と同名アイテムが見つかれば、再選択する
    for (int row = 0; row < m_model.rowCount(); row++) {
        QModelIndex index = m_model.index(row, 1);
        if (m_model.fileInfo(index).fileName().indexOf(m_saveName) != -1) {
            setCurrentIndex(index);
            return;
        }
    }

    // 同名アイテムが見つからなければ、行を維持する
    if (m_saveRow >= m_model.rowCount()) {
        m_saveRow = m_model.rowCount() - 1;
    }
    setCurrentIndex(m_model.index(m_saveRow, 1));
}

void FolderView::searchItem(const QString &text)
{
    qDebug() << side() << "searchItem" << text;

    for (int row = 0; row < m_model.rowCount(); row++) {
        QModelIndex index = m_model.index(row, 1);
        QString name = m_model.fileInfo(index).fileName().toLower();
        if (name.indexOf(text.toLower()) != -1) {
            setCurrentIndex(index);
            emit itemFound();
            return;
        }
    }

    emit itemNotFound();
}

void FolderView::searchNext(const QString &text)
{
    qDebug() << side() << "searchNext" << text;

    for (int row = currentIndex().row() + 1; row < m_model.rowCount(); row++) {
        QModelIndex index = m_model.index(row, 1);
        QString name = m_model.fileInfo(index).fileName().toLower();
        if (name.indexOf(text.toLower()) != -1) {
            setCurrentIndex(index);
            emit itemFound();
            return;
        }
    }

    emit itemNotFound();
}

void FolderView::searchPrev(const QString &text)
{
    qDebug() << side() << "searchPrev" << text;

    for (int row = currentIndex().row() - 1; row >= 0; row--) {
        QModelIndex index = m_model.index(row, 1);
        QString name = m_model.fileInfo(index).fileName().toLower();
        if (name.indexOf(text.toLower()) != -1) {
            setCurrentIndex(index);
            emit itemFound();
            return;
        }
    }

    emit itemNotFound();
}

void FolderView::setCheckStateAll(Qt::CheckState state)
{
    qDebug() << side() << "setCheckStateAll();" << state;

    QModelIndex current = currentIndex();

    for (int n = 0; n < m_model.rowCount(); n++) {
        m_model.setData(m_model.index(n, 0), state, Qt::CheckStateRole);
    }

    setCurrentIndex(current);
}

void FolderView::setCheckStateAllFiles()
{
    qDebug() << side() << "setCheckStateAllFiles();";

    QModelIndex current = currentIndex();

    for (int n = 0; n < m_model.rowCount(); n++) {
        QModelIndex index = m_model.index(n, 0);
        QFileInfo info = m_model.fileInfo(index);
        if (info.isDir()) {
            m_model.setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
        else {
            m_model.setData(index, Qt::Checked, Qt::CheckStateRole);
        }
    }

    setCurrentIndex(current);
}

void FolderView::invertCheckState()
{
    qDebug() << side() << "invertCheckState();";

    QModelIndex current = currentIndex();

    for (int n = 0; n < m_model.rowCount(); n++) {
        QModelIndex index = m_model.index(n, 0);
        if (m_model.data(index, Qt::CheckStateRole).toInt() == Qt::Checked) {
            m_model.setData(index, Qt::Unchecked, Qt::CheckStateRole);
        }
        else {
            m_model.setData(index, Qt::Checked, Qt::CheckStateRole);
        }
    }

    setCurrentIndex(current);
}

void FolderView::toggleCheckState(const QModelIndex &index)
{
    qDebug() << side() << "toggleCheckState();" << index;

    QFileInfo info = m_model.fileInfo(index);
    if (info.fileName() != "..") {
        QModelIndex chkIndex = m_model.index(index.row(), 0);
        if (m_model.data(chkIndex, Qt::CheckStateRole).toInt() == Qt::Checked) {
            m_model.setData(chkIndex, Qt::Unchecked, Qt::CheckStateRole);
        }
        else {
            m_model.setData(chkIndex, Qt::Checked, Qt::CheckStateRole);
        }
    }
    // 最終行でなければ、次のアイテムに移動する
    if (index.row() == m_model.rowCount() - 1) {
        setCurrentIndex(index);
    }
    else if (index.row() < m_model.rowCount() - 1) {
        setCurrentIndex(m_model.index(index.row() + 1, 1));
    }
}

bool FolderView::historyBack()
{
    if (!m_history.isBegin()) {
        setPath(m_history.back(), false);
        return true;
    }
    return false;
}

bool FolderView::historyForward()
{
    if (!m_history.isEnd()) {
        setPath(m_history.forward(), false);
        return true;
    }
    return false;
}

QFileInfo FolderView::currentItem() const
{
    qDebug() << side() << "currentItem";

    return m_model.fileInfo(currentIndex());
}

QFileInfoList FolderView::checkedItems() const
{
    qDebug() << side() << "checkedItems()";

    QFileInfoList list;
    for (int n = 0; n < m_model.rowCount(); n++) {
        QModelIndex index = m_model.index(n, 0);
        if (m_model.data(index, Qt::CheckStateRole).toInt() == Qt::Checked) {
            list << m_model.fileInfo(index);
        }
    }
    return list;
}

QFileInfoList FolderView::selectedItems() const
{
    qDebug() << side() << "selectedItems";

    // マークされているアイテムを取得する
    QFileInfoList list = checkedItems();

    // 一つもマークされていなければ、カーソル位置のアイテムを取得する
    if (list.isEmpty()) {
        list << currentItem();
    }

    return list;
}

FolderPanel *FolderView::parentPanel() const
{
    return qobject_cast<FolderPanel*>(parentWidget());
}

void FolderView::setPath(const QString &path, bool addHistory)
{
    qDebug() << side() << "setPath" << path << addHistory;

    Q_ASSERT(QFileInfo(path).isDir());

    m_mainWnd->statusBar()->showMessage(tr("ファイルリストを取得しています..."));
    emit retrieveStarted(path);

    m_model.setPath(path);
    setCurrentIndex(m_model.index(0, 1));

    if (addHistory) {
        m_history.add(path);
    }

    m_mainWnd->statusBar()->showMessage(tr("レディ"));
}

void FolderView::setFilter(QDir::Filters filter, bool enable)
{
    qDebug() << side() << "setFilter" << enable;

    if (enable) {
        m_model.setFilter(m_model.filter() | filter);
    }
    else {
        m_model.setFilter(m_model.filter() ^ filter);
    }
}

void FolderView::setHistoryIndexAt(int index)
{
    m_history.setAt(index);
    setPath(m_history.current(), false);
}

void FolderView::setNameFilters(const QStringList &list)
{
    m_model.setNameFilters(list);
}

void FolderView::setSorting()
{
    qDebug() << side() << "setSorting();";

    QSettings settings;
    QDir::SortFlags flags;

    int sortBy = settings.value(side() + slash + IniKey_SortBy).toInt();
    switch (sortBy) {
    case SortByDate:    flags |= QDir::Time; break;
    case SortBySize:    flags |= QDir::Size; break;
    case SortByType:    flags |= QDir::Type; break;
    default:            flags |= QDir::Name; break;
    }

    // デフォルトだと文字列は昇順で、数値は降順…orz
    int orderBy = settings.value(side() + slash + IniKey_OrderBy).toInt();
    if (((sortBy == SortByName || sortBy == SortByType) && orderBy == OrderByDesc) ||
        ((sortBy == SortByDate || sortBy == SortBySize) && orderBy == OrderByAsc))
    {
        flags |= QDir::Reversed;
    }

    switch (settings.value(side() + slash + IniKey_PutDirs).toInt()) {
    case PutDirsFirst:  flags |= QDir::DirsFirst; break;
    case PutDirsLast:   flags |= QDir::DirsLast; break;
    }

    if (settings.value(side() + slash + IniKey_IgnoreCase).toBool()) {
        flags |= QDir::IgnoreCase;
    }

    m_model.setSorting(flags);
}

void FolderView::keyPressEvent(QKeyEvent *event)
{
    qDebug() << side() << "keyPressEvent";

    if (IsKeyUpDown(event)) {
        QTableView::keyPressEvent(event);
        event->accept();
        return;
    }

    // MainWindowへ
    event->ignore();
}

void FolderView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    qDebug() << side() << "dataChanged();";

    emit dataChanged();

    QTableView::dataChanged(topLeft, bottomRight, roles);
}

void FolderView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    qDebug() << side() << "currentChanged();" << current;

    emit currentChanged(m_model.fileInfo(current));

    QTableView::currentChanged(current, previous);
}

void FolderView::mousePressEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) || (event->buttons() & Qt::RightButton))
    {
        m_dragStartPos = event->pos();
    }

    QTableView::mousePressEvent(event);
}

void FolderView::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton) &&
            !(event->buttons() & Qt::RightButton))
    {
        QTableView::mouseMoveEvent(event);
        return;
    }
    if ((event->pos() - m_dragStartPos).manhattanLength()
            < QApplication::startDragDistance())
    {
        QTableView::mouseMoveEvent(event);
        return;
    }

    QFileInfoList list = selectedItems();
    if (list.isEmpty()) {
        QTableView::mousePressEvent(event);
        return;
    }

    QList<QUrl> urls;
    foreach (const QFileInfo &info, list) {
        urls << QUrl::fromLocalFile(info.absoluteFilePath());
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);

    m_dragging = true;
    if (event->buttons() & Qt::RightButton) {
        drag->exec(Qt::CopyAction | Qt::MoveAction);
    }
    else {
        drag->exec(Qt::CopyAction);
    }
    m_dragging = false;
}

void FolderView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
        return;
    }

    QTableView::dragEnterEvent(event);
}

void FolderView::dropEvent(QDropEvent *event)
{
    qDebug() << side() << "dropEvent();";

    if (m_dragging) {
        event->ignore();
        return;
    }

    QFileInfoList list;
    foreach (const QUrl &url, event->mimeData()->urls()) {
        QString path = QFileInfo(url.toLocalFile()).canonicalFilePath();
        if (!path.isEmpty()) {
            list << path;
        }
        else {
            qDebug() << "path is empty." << url;
        }
    }

    if (list.isEmpty()) {
        event->ignore();
        return;
    }

    event->acceptProposedAction();

    emit dropAccepted(list, event);
}


void FolderView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << side() << "contextMenuEvent();";

    emit requestContextMenu(event);
}
