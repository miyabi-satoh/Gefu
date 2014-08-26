#include "common.h"
#include "folderview.h"

#include <QDebug>
#include <QHeaderView>
#include <QKeyEvent>
#include <QSettings>
#include <QMimeData>
#include <QApplication>
#include <QDrag>
#include <QMenu>

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
    m_model(),
    m_dragStartPos(),
    m_dragging(false)
{
    setModel(&m_model);
    connect(&m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged(QModelIndex,QModelIndex)));

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

void FolderView::initialize()
{
    qDebug() << side() << "initialize";

    setColumnWidth(0, 30);

    QHeaderView *header;
    // 列のリサイズモードを設定する
    header = horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Stretch);

    // 前回終了時のパスを開く
    QSettings settings;
    setPath(settings.value(side() + slash + IniKey_Dir).toString(), true);

    // 色とフォントを設定する
    updateAppearance();
}

void FolderView::updateAppearance()
{
    qDebug() << side() << "updateAppearance";
    m_model.updateAppearance();

    QSettings settings;
    QPalette pal = palette();
    pal.setColor(QPalette::Base, settings.value(IniKey_ViewColorBgNormal).value<QColor>());
    setPalette(pal);

    // 行の高さを設定する
    QHeaderView *header = verticalHeader();
    header->setDefaultSectionSize(QFontMetrics(m_model.font()).height() * 1.5);
}

void FolderView::refresh()
{
    qDebug() << side() << "refresh";

    int row = currentIndex().row();

    setPath(m_model.absolutePath(), false);

    if (row < 0) {
        row = 0;
    }
    else if (row >= m_model.rowCount()) {
        row = m_model.rowCount() - 1;
    }
    setCurrentIndex(m_model.index(row, 0));
    selectRow(row);
}

void FolderView::searchItem(const QString &text)
{
    qDebug() << "FolderView::searchItem" << text;

    for (int row = 0; row < m_model.rowCount(); row++) {
        QModelIndex index = m_model.index(row, 0);
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
    qDebug() << "FolderView::searchNext" << text;

    for (int row = currentIndex().row() + 1; row < m_model.rowCount(); row++) {
        QModelIndex index = m_model.index(row, 0);
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
        QModelIndex index = m_model.index(row, 0);
        QString name = m_model.fileInfo(index).fileName().toLower();
        if (name.indexOf(text.toLower()) != -1) {
            setCurrentIndex(index);
            emit itemFound();
            return;
        }
    }

    emit itemNotFound();
}

void FolderView::setCheckStateAll(bool checked)
{
    qDebug() << side() << "markAll();";

    QModelIndex current = currentIndex();

    setUpdatesEnabled(false);
    m_model.setCheckStateAll(checked ? Qt::Checked : Qt::Unchecked);
    setUpdatesEnabled(true);

    setCurrentIndex(current);
}

void FolderView::setCheckStateAllFiles()
{
    qDebug() << side() << "markAllFiles();";

    QModelIndex current = currentIndex();

    setUpdatesEnabled(false);
    for (int n = 0; n < m_model.rowCount(); n++) {
        QModelIndex index = m_model.index(n, 0);
        QFileInfo info = m_model.fileInfo(index);
        if (info.isDir()) {
            m_model.setCheckState(index, Qt::Unchecked);
        }
        else {
            m_model.setCheckState(index, Qt::Checked);
        }
    }
    setUpdatesEnabled(true);

    setCurrentIndex(current);
}

void FolderView::invertCheckState()
{
    qDebug() << side() << "invertCheckState();";

    QModelIndex current = currentIndex();

    setUpdatesEnabled(false);
    for (int n = 0; n < m_model.rowCount(); n++) {
        QModelIndex index = m_model.index(n, 0);
        if (m_model.checkState(index) == Qt::Checked) {
            m_model.setCheckState(index, Qt::Unchecked);
        }
        else {
            m_model.setCheckState(index, Qt::Checked);
        }
    }
    setUpdatesEnabled(true);

    setCurrentIndex(current);
}

void FolderView::toggleCheckState(const QModelIndex &index)
{
    QFileInfo info = m_model.fileInfo(index);
    if (info.fileName() != "..") {
        if (m_model.checkState(index) == Qt::Checked) {
            m_model.setCheckState(index, Qt::Unchecked);
        }
        else {
            m_model.setCheckState(index, Qt::Checked);
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
    qDebug() << "FolderView::currentItem";

    Q_ASSERT(currentIndex().isValid());

    return m_model.fileInfo(currentIndex());
}

QFileInfoList FolderView::checkedItems() const
{
    qDebug() << side() << "checkedItems()";

    return m_model.checkedItems();
}

QFileInfoList FolderView::selectedItems() const
{
    qDebug() << side() << "selectedItems";

    // マークされているアイテムを取得する
    QFileInfoList list = m_model.checkedItems();

    // 一つもマークされていなければ、カーソル位置のアイテムを取得する
    if (list.isEmpty()) {
        list << currentItem();
    }

    return list;
}

void FolderView::setPath(const QString &path, bool addHistory)
{
    qDebug() << side() << "setPath" << path << addHistory;

    Q_ASSERT(QFileInfo(path).isDir());

    emit retrieveStarted(path);

    setUpdatesEnabled(false);
    m_model.setPath(path);
    setCurrentIndex(m_model.index(0, 0));
    setUpdatesEnabled(true);

    if (addHistory) {
        m_history.add(path);
    }

    emit retrieveFinished();
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
    refresh();
}

void FolderView::setSorting(QDir::SortFlags flags)
{
    m_model.setSorting(flags);
    refresh();
}

void FolderView::keyPressEvent(QKeyEvent *event)
{
    qDebug() << side() << "keyPressEvent";

    emit keyPressed(event);

    if (!event->isAccepted()) {
        QTableView::keyPressEvent(event);
    }
    else {
        qDebug() << "KeyEvent accepted.";
    }
}

void FolderView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    emit dataChanged();

    QTableView::dataChanged(topLeft, bottomRight, roles);
}

void FolderView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
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

    emit dropAccepted(list);
}


void FolderView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << side() << "contextMenuEvent();";

    emit requestContextMenu(event);
}
