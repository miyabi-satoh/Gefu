#include "common.h"
#include "copymoveworker.h"
#include "deleteworker.h"
#include "filetablemodel.h"
#include "filetableview.h"
#include "historydialog.h"
#include "mainwindow.h"
#include "operationdialog.h"
#include "overwritedialog.h"
#include "renamemultidialog.h"
#include "renamesingledialog.h"
#include "renameworker.h"
#include "sortdialog.h"

#include <QDebug>
#include <QDesktopServices>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>
#include <QAction>
#include <QSettings>
#include <QFileDialog>
#include <QKeyEvent>
#include <QStatusBar>

#define CHECK_FOCUS {                   \
        if (!hasFocus()) { return; }    \
    }

#define MENU_TRRIGGERED(x) \
    getMainWnd()->findChild<QAction*>(#x), SIGNAL(triggered())
#define MENU_TOGGLED(x) \
    getMainWnd()->findChild<QAction*>(#x), SIGNAL(toggled(bool))

FileTableView::FileTableView(QWidget *parent) :
    QTableView(parent),
    m_side(),
    m_history()
{
    // シグナル/スロットを設定する
    connect(MENU_TRRIGGERED(action_Open), this, SLOT(setPath()));
    connect(MENU_TRRIGGERED(action_Exec), this, SLOT(openUrl()));
    connect(MENU_TRRIGGERED(action_OpenEditor), this, SLOT(XXX()));
    connect(MENU_TRRIGGERED(action_Command), this, SLOT(kickProcess()));

    connect(MENU_TRRIGGERED(mark_Toggle), this, SLOT(toggleChecked()));
    connect(MENU_TRRIGGERED(mark_All), this, SLOT(checkAllItems()));
    connect(MENU_TRRIGGERED(mark_AllFiles), this, SLOT(checkAllFiles()));
    connect(MENU_TRRIGGERED(mark_AllOff), this, SLOT(uncheckAllItems()));
    connect(MENU_TRRIGGERED(mark_Invert), this, SLOT(invertAllChecked()));

    connect(MENU_TRRIGGERED(view_FromOther), this, SLOT(setPathFromOther()));
    connect(MENU_TRRIGGERED(view_ToOther), this, SLOT(setPathToOther()));
    connect(MENU_TRRIGGERED(view_Swap), this, SLOT(swapPath()));
    connect(MENU_TRRIGGERED(view_Sort), this, SLOT(setSort()));
    connect(MENU_TRRIGGERED(view_Refresh), this, SLOT(refresh()));

    connect(MENU_TRRIGGERED(move_Back), this, SLOT(back()));
    connect(MENU_TRRIGGERED(move_Forward), this, SLOT(forward()));
    connect(MENU_TRRIGGERED(move_History), this, SLOT(showHidtory()));
    connect(MENU_TRRIGGERED(move_Home), this, SLOT(jumpToHome()));
    connect(MENU_TRRIGGERED(move_Parent), this, SLOT(jumpToParent()));
    connect(MENU_TRRIGGERED(move_Root), this, SLOT(jumpToRoot()));
    connect(MENU_TRRIGGERED(move_Jump), this, SLOT(jumpTo()));
    connect(MENU_TRRIGGERED(move_Down), this, SLOT(cursorDown()));
    connect(MENU_TRRIGGERED(move_Up), this, SLOT(cursorUp()));
    connect(MENU_TRRIGGERED(move_Begin), this, SLOT(cursorToBegin()));
    connect(MENU_TRRIGGERED(move_End), this, SLOT(cursorToEnd()));

    connect(MENU_TRRIGGERED(cmd_Copy), this, SLOT(cmdCopy()));
    connect(MENU_TRRIGGERED(cmd_Move), this, SLOT(cmdMove()));
    connect(MENU_TRRIGGERED(cmd_Delete), this, SLOT(cmdDelete()));
    connect(MENU_TRRIGGERED(cmd_Rename), this, SLOT(cmdRename()));
    connect(MENU_TRRIGGERED(cmd_NewFile), this, SLOT(newFile()));
    connect(MENU_TRRIGGERED(cmd_NewFolder), this, SLOT(newFolder()));

    connect(getMainWnd(), SIGNAL(showHiddenFiles(bool)),
            this, SLOT(showHiddenFiles(bool)));
    connect(getMainWnd(), SIGNAL(showSystemFiles(bool)),
            this, SLOT(showSystemFiles(bool)));

    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(setRootIndex(QModelIndex)));
}

FileTableView::~FileTableView()
{
    QSettings settings;
    FileTableModel *m = static_cast<FileTableModel*>(model());

    settings.setValue(side() + slash + IniKey_Dir, m->absolutePath());
}

QString FileTableView::side() const
{
    return m_side;
}

void FileTableView::setSide(const QString &side)
{
    m_side = side;

    // コンストラクタではステータスバーが生成されていないため
    // このタイミングでconnectする
    connect(this, SIGNAL(indexChanged(QString)),
            getMainWnd()->statusBar(), SLOT(showMessage(QString)));
}

void FileTableView::setRootPath(const QString &path, bool addHistory)
{
    FileTableModel *m = static_cast<FileTableModel*>(model());
    QFileInfo info(path);

    if (info.isDir()) {
        getMainWnd()->statusBar()->showMessage(tr("ファイルリストの取得中..."));

        setUpdatesEnabled(false);
        m->setPath(info.absoluteFilePath());
        setUpdatesEnabled(true);

        if (addHistory) {
            m_history.add(info.absoluteFilePath());
        }
        updateMenu();

        getMainWnd()->statusBar()->showMessage(tr("レディ"), 5000);
    }
}

QFileInfoList FileTableView::selectedItems() const
{
    FileTableModel *m = static_cast<FileTableModel*>(model());
    QFileInfoList list = m->checkedItems();
    if (list.isEmpty()) {
        QFileInfo info = m->fileInfo(currentIndex());
        if (info.fileName() != "..") {
            list.append(info);
        }
    }

    return list;
}

void FileTableView::updateMenu()
{
    if (m_history.isEmpty()) {
        getMainWnd()->findChild<QAction*>("move_Back")->setEnabled(false);
        getMainWnd()->findChild<QAction*>("move_Forward")->setEnabled(false);

    }
    else {
        getMainWnd()->findChild<QAction*>("move_Back")
                ->setEnabled(!m_history.isBegin());
        getMainWnd()->findChild<QAction*>("move_Forward")
                ->setEnabled(!m_history.isEnd());
    }
}

void FileTableView::setPath()
{
    CHECK_FOCUS;

    setRootIndex(currentIndex());
}

void FileTableView::openUrl()
{
    CHECK_FOCUS;

    QFileInfoList list = selectedItems();
    foreach (const QFileInfo &info, list) {
        QString path = QDir::toNativeSeparators(info.absoluteFilePath());
        QDesktopServices::openUrl(QUrl("file:///" + path));
    }
}

void FileTableView::kickProcess()
{
    CHECK_FOCUS;

    QFileInfoList list = selectedItems();
    QString command = QString::null;
    foreach (const QFileInfo &info, list) {
        if (info.isExecutable()) {
            command = QQ(info.fileName()) + " " + command;
        }
        else {
            command += " " + QQ(info.fileName());
        }
    }

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setWindowTitle(tr("コマンドを実行"));
    dlg.setLabelText(tr("コマンド："));
    dlg.setTextValue(command);

    QSize szMainWnd = getMainWnd()->size();
    QSize szDialog = dlg.size();
    szDialog.setWidth(szMainWnd.width() * 0.8);
    dlg.resize(szDialog);

    int ret = dlg.exec();
    command = dlg.textValue();
    if (ret == QDialog::Accepted && !command.isEmpty()) {
        FileTableModel *m = static_cast<FileTableModel*>(model());
        QProcess process(this);
        process.setWorkingDirectory(m->absolutePath());
        if (!process.startDetached(command)) {
            QMessageBox::critical(
                        this, tr("エラー"),
                        tr("コマンドの実行に失敗しました。<br/>") + command);
        }
    }
}

void FileTableView::toggleChecked()
{
    CHECK_FOCUS;

    FileTableModel *m = static_cast<FileTableModel*>(model());

    QModelIndex index = currentIndex();

    QFileInfo info = m->fileInfo(index);
    if (info.fileName() != "..") {
        if (m->checkState(index) == Qt::Checked) {
            m->setCheckState(index, Qt::Unchecked);
        }
        else {
            m->setCheckState(index, Qt::Checked);
        }
    }
    // 最終行でなければ、次のアイテムに移動する
    if (index.row() == m->rowCount() - 1) {
        setCurrentIndex(index);
    }
    else if (index.row() < m->rowCount() - 1) {
        qDebug() << "set = " << index.row() + 1;
        setCurrentIndex(m->index(index.row() + 1, 1));
    }
}

void FileTableView::checkAllItems()
{
    CHECK_FOCUS;

    QModelIndex index = currentIndex();
    FileTableModel *m = static_cast<FileTableModel*>(model());
    setUpdatesEnabled(false);
    m->setCheckStateAll(Qt::Checked);
    setUpdatesEnabled(true);
    setCurrentIndex(index);
}

void FileTableView::checkAllFiles()
{
    CHECK_FOCUS;

    QModelIndex index = currentIndex();
    FileTableModel *m = static_cast<FileTableModel*>(model());
    setUpdatesEnabled(false);
    for (int n = 0; n < m->rowCount(); n++) {
        QModelIndex index = m->index(n, 0);
        QFileInfo info = m->fileInfo(index);
        if (info.isDir()) {
            m->setCheckState(index, Qt::Unchecked);
        }
        else {
            m->setCheckState(index, Qt::Checked);
        }
        update(index);
    }
    setUpdatesEnabled(true);
    setCurrentIndex(index);
}

void FileTableView::uncheckAllItems()
{
    CHECK_FOCUS;

    QModelIndex index = currentIndex();
    FileTableModel *m = static_cast<FileTableModel*>(model());
    setUpdatesEnabled(false);
    m->setCheckStateAll(Qt::Unchecked);
    setUpdatesEnabled(true);
    setCurrentIndex(index);
}

void FileTableView::invertAllChecked()
{
    CHECK_FOCUS;

    QModelIndex index = currentIndex();
    FileTableModel *m = static_cast<FileTableModel*>(model());
    setUpdatesEnabled(false);
    for (int n = 0; n < m->rowCount(); n++) {
        QModelIndex index = m->index(n, 0);
        if (m->checkState(index) == Qt::Checked) {
            m->setCheckState(index, Qt::Unchecked);
        }
        else {
            m->setCheckState(index, Qt::Checked);
        }
        update(index);
    }
    setUpdatesEnabled(true);
    setCurrentIndex(index);
}

void FileTableView::setPathFromOther()
{
    CHECK_FOCUS;

    FileTableView *other = getMainWnd()->otherSideView(this);
    if (other == NULL) {
        return;
    }

    FileTableModel *mOther = static_cast<FileTableModel*>(other->model());

    setRootPath(mOther->absolutePath(), true);
}

void FileTableView::setPathToOther()
{
    CHECK_FOCUS;

    FileTableView *other = getMainWnd()->otherSideView(this);
    if (other == NULL) {
        return;
    }

    FileTableModel *m = static_cast<FileTableModel*>(model());

    other->setRootPath(m->absolutePath(), true);
}

void FileTableView::swapPath()
{
    CHECK_FOCUS;

    FileTableView *other = getMainWnd()->otherSideView(this);
    if (other == NULL) {
        return;
    }

    FileTableModel *m = static_cast<FileTableModel*>(model());
    FileTableModel *mOther = static_cast<FileTableModel*>(other->model());

    QString path = m->absolutePath();
    QString pathOther = mOther->absolutePath();

    setRootPath(pathOther, true);
    other->setRootPath(path, true);
}

void FileTableView::showHiddenFiles(bool show)
{
    FileTableModel *m = static_cast<FileTableModel*>(model());
    if (show) {
        m->setFilter(m->filter() | QDir::Hidden);
    }
    else {
        m->setFilter(m->filter() ^ QDir::Hidden);
    }

    setUpdatesEnabled(false);
    refresh();
    setUpdatesEnabled(true);
}

void FileTableView::showSystemFiles(bool show)
{
    FileTableModel *m = static_cast<FileTableModel*>(model());
    if (show) {
        m->setFilter(m->filter() | QDir::System);
    }
    else {
        m->setFilter(m->filter() ^ QDir::System);
    }

    setUpdatesEnabled(false);
    refresh();
    setUpdatesEnabled(true);
}

void FileTableView::setSort()
{
    CHECK_FOCUS;

    SortDialog dlg(this);
    dlg.setRightOrLeft(side());
    if (dlg.exec() == QDialog::Accepted) {
        QSettings settings;
        FileTableModel *m = static_cast<FileTableModel*>(model());
        m->setSorting(QDir::Name);  // 0

        int sortBy = settings.value(side() + slash + IniKey_SortBy, 0).toInt();
        switch (sortBy) {
        case SortByDate:    m->setSorting(m->sorting() | QDir::Time); break;
        case SortBySize:    m->setSorting(m->sorting() | QDir::Size); break;
        case SortByType:    m->setSorting(m->sorting() | QDir::Type); break;
        default:            m->setSorting(m->sorting() | QDir::Name); break;
        }

        // デフォルトだと文字列は昇順で、数値は降順…orz
        int orderBy = settings.value(side() + slash + IniKey_OrderBy, 0).toInt();
        if (((sortBy == SortByName || sortBy == SortByType) && orderBy == OrderByDesc) ||
            ((sortBy == SortByDate || sortBy == SortBySize) && orderBy == OrderByAsc))
        {
            m->setSorting(m->sorting() | QDir::Reversed);
        }

        switch (settings.value(side() + slash + IniKey_PutDirs, 0).toInt()) {
        case PutDirsFirst:  m->setSorting(m->sorting() | QDir::DirsFirst); break;
        case PutDirsLast:   m->setSorting(m->sorting() | QDir::DirsLast); break;
        }

        if (settings.value(side() + slash + IniKey_IgnoreCase, true).toBool()) {
            m->setSorting(m->sorting() | QDir::IgnoreCase);
        }

        setUpdatesEnabled(false);
        refresh();
        setUpdatesEnabled(true);
    }
}

void FileTableView::refresh()
{
    FileTableModel *m = static_cast<FileTableModel*>(model());
    int row = currentIndex().row();
    setRootPath(m->absolutePath(), false);
    if (m->rowCount() <= row) {
        row = m->rowCount() - 1;
    }
    setCurrentIndex(m->index(row, 0));
    selectRow(row);
}

void FileTableView::back()
{
    CHECK_FOCUS;

    if (!m_history.isBegin()) {
        const QString &path = m_history.back();
        setRootPath(path, false);

        updateMenu();
    }
}

void FileTableView::forward()
{
    CHECK_FOCUS;

    if (!m_history.isEnd()) {
        const QString &path = m_history.forward();
        setRootPath(path, false);

        updateMenu();
    }
}

void FileTableView::showHidtory()
{
    CHECK_FOCUS;

    FileTableView *other = getMainWnd()->otherSideView(this);

    HistoryDialog dlg(this);
    if (side() == "Left") {
        dlg.setDefaultLeft(true);
        dlg.setHistory(&m_history, other->history());
    }
    else {
        dlg.setDefaultLeft(false);
        dlg.setHistory(other->history(), &m_history);
    }

    QSize szMainWnd = getMainWnd()->size();
    QSize szDialog = dlg.size();
    szDialog.setWidth(szMainWnd.width() * 0.8);
    dlg.resize(szDialog);
    if (dlg.exec() == QDialog::Accepted) {
        if (side() == dlg.selectedSide()) {
            m_history.setAt(dlg.selectedIndex());
            setRootPath(m_history.current(), false);
        }
        else {
            setRootPath(other->history()->at(dlg.selectedIndex()), true);
        }
        updateMenu();
        qDebug() << dlg.selectedIndex();
    }
}

void FileTableView::jumpToHome()
{
    CHECK_FOCUS;

    setRootPath(QDir::homePath(), true);
}

void FileTableView::jumpToParent()
{
    CHECK_FOCUS;

    FileTableModel *m = static_cast<FileTableModel*>(model());
    QDir dir(m->absolutePath());
    dir.cdUp();

    setRootPath(dir.absolutePath(), true);
}

void FileTableView::jumpToRoot()
{
    CHECK_FOCUS;

    setRootPath(QDir::rootPath(), true);
}

void FileTableView::jumpTo()
{
    CHECK_FOCUS;

    FileTableModel *m = static_cast<FileTableModel*>(model());
    QString path = QFileDialog::getExistingDirectory(
                this, tr("フォルダを選択"), m->absolutePath());
    if (!path.isEmpty()) {
        setRootPath(path, true);
    }
}

void FileTableView::cursorDown()
{
    CHECK_FOCUS;

    int row = currentIndex().row();
    if (row < model()->rowCount() - 1) {
        setCurrentIndex(model()->index(row + 1, 1));
    }
}

void FileTableView::cursorUp()
{
    CHECK_FOCUS;

    int row = currentIndex().row();
    if (row > 0) {
        setCurrentIndex(model()->index(row - 1, 1));
    }
}

void FileTableView::cursorToBegin()
{
    CHECK_FOCUS;

    setCurrentIndex(model()->index(0, 1));
}

void FileTableView::cursorToEnd()
{
    CHECK_FOCUS;

    setCurrentIndex(model()->index(model()->rowCount() - 1, 1));
}

void FileTableView::cmdCopy()
{
    CHECK_FOCUS;

    QFileInfoList list = selectedItems();
    if (list.isEmpty()) {
        return;
    }

    FileTableView *other = getMainWnd()->otherSideView(this);
    FileTableModel *mOther = static_cast<FileTableModel*>(other->model());
    CopyMoveWorker *worker = new CopyMoveWorker();
    connect(worker, SIGNAL(askOverWrite(bool*,int*,int*,QString*,QString,QString)),
            this, SLOT(askOverWrite(bool*,int*,int*,QString*,QString,QString)));
    worker->setCopyList(&list);
    worker->setTargetDir(mOther->absolutePath());
    worker->setMoveMode(false);

    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("コピー"));
    opDlg.setWorker(worker);

    opDlg.exec();
}

void FileTableView::cmdMove()
{
    CHECK_FOCUS;

    QFileInfoList list = selectedItems();
    if (list.isEmpty()) {
        return;
    }

    FileTableView *other = getMainWnd()->otherSideView(this);
    FileTableModel *mOther = static_cast<FileTableModel*>(other->model());
    CopyMoveWorker *worker = new CopyMoveWorker();
    connect(worker, SIGNAL(askOverWrite(bool*,int*,int*,QString*,QString,QString)),
            this, SLOT(askOverWrite(bool*,int*,int*,QString*,QString,QString)));
    worker->setCopyList(&list);
    worker->setTargetDir(mOther->absolutePath());
    worker->setMoveMode(true);

    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("移動"));
    opDlg.setWorker(worker);

    opDlg.exec();
}

void FileTableView::cmdDelete()
{
    CHECK_FOCUS;

    QFileInfoList list = selectedItems();
    if (list.isEmpty()) {
        return;
    }

    QString msg;
    if (list.size() == 1) {
        msg = list[0].fileName();
    }
    else {
        msg = tr("%1個のアイテム").arg(list.size());
    }
    int ret = QMessageBox::question(
                this, tr("確認"),
                msg + tr("を削除します<br/>よろしいですか？"));
    if (ret == QMessageBox::Yes) {
        DeleteWorker *worker = new DeleteWorker();
        worker->setDeleteList(&list);

        OperationDialog opDlg(this);
        opDlg.setWindowTitle(tr("削除"));
        opDlg.setWorker(worker);
        opDlg.exec();
    }
}

void FileTableView::cmdRename()
{
    CHECK_FOCUS;

    QFileInfoList list = selectedItems();
    if (list.isEmpty()) {
        return;
    }

    IRenameDialog *dlg;
    if (list.size() == 1) {
        dlg = new RenameSingleDialog(this);
    }
    else {
        dlg = new RenameMultiDialog(this);
    }
    FileTableModel *m = static_cast<FileTableModel*>(model());
    dlg->setWorkingDirectory(m->absolutePath());
    dlg->setNames(list);
    int dlgResult = dlg->exec();
    if (dlgResult == QDialog::Accepted && !dlg->renameMap().isEmpty()) {
        RenameWorker *worker = new RenameWorker();
        worker->setRenameMap(&dlg->renameMap());

        OperationDialog opDlg(this);
        opDlg.setWindowTitle(tr("名前を変更"));
        opDlg.setWorker(worker);
        opDlg.exec();
    }
}

void FileTableView::newFile()
{
    CHECK_FOCUS;

    bool bOk;
    QString name = QInputDialog::getText(
                this, tr("ファイルを作成"), tr("ファイル名："),
                QLineEdit::Normal, "", &bOk);
    if (bOk && !name.isEmpty()) {
        FileTableModel *m = static_cast<FileTableModel*>(model());
        QDir dir(m->absolutePath());
        QFile file(dir.absoluteFilePath(name));
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(
                        this, tr("エラー"),
                        tr("ファイルの作成に失敗しました。"));
        }
        else {
            file.close();
        }
    }
}

void FileTableView::newFolder()
{
    CHECK_FOCUS;

    bool bOk;
    QString name = QInputDialog::getText(
                this, tr("フォルダを作成"), tr("フォルダ名："),
                QLineEdit::Normal, "", &bOk);
    if (bOk && !name.isEmpty()) {
        FileTableModel *m = static_cast<FileTableModel*>(model());
        QDir dir(m->absolutePath());
        if (!dir.mkpath(name)) {
            QMessageBox::critical(
                        this, tr("エラー"),
                        tr("フォルダの作成に失敗しました。"));
        }
    }
}

void FileTableView::XXX()
{
    CHECK_FOCUS;

    qDebug() << sender()->objectName() << "に対するスロットは未実装です。";
}

void FileTableView::askOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod,
                                 QString *alias, const QString &srcPath,
                                 const QString &tgtPath)
 {
     OverWriteDialog dlg;
     dlg.setCopyMethod(*prevCopyMethod);
     dlg.setSameMethodChecked(*copyMethod != OverWriteDialog::Undefined);
     dlg.setFileInfo(srcPath, tgtPath);
     if (dlg.exec() == QDialog::Rejected) {
         *bOk = false;
     }
     else {
         *prevCopyMethod = dlg.copyMethod();
         if (dlg.isSameMethodChecked()) {
             *copyMethod = *prevCopyMethod;
         }
         *alias = dlg.alias();
         *bOk = true;
     }
     CopyMoveWorker *worker = static_cast<CopyMoveWorker*>(sender());
     worker->endAsking();
 }

void FileTableView::setRootIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    FileTableModel *m = static_cast<FileTableModel*>(model());

    if (m->isDir(index)) {
        setRootPath(m->absoluteFilePath(index), true);
    }
}

void FileTableView::keyPressEvent(QKeyEvent *event)
{
    // Macでアクションが処理されないケースがあるため、
    // キーイベントを拾ってアクションシグナルを起動する
    QString modifier = QString::null;
    if (event->modifiers() & Qt::ShiftModifier)     { modifier += "Shift+"; }
    if (event->modifiers() & Qt::ControlModifier)   { modifier += "Ctrl+"; }
    if (event->modifiers() & Qt::AltModifier)       { modifier += "Alt+"; }
    if (event->modifiers() & Qt::MetaModifier)      { modifier += "Meta+"; }

    QString key = QKeySequence(event->key()).toString();
    QString ksq = QKeySequence(modifier + key).toString();

    if (!ksq.isEmpty()) {
        foreach (QObject *obj, getMainWnd()->children()) {
            QAction *action = qobject_cast<QAction*>(obj);
            if (action) {
                foreach (const QKeySequence &keySeq, action->shortcuts()) {
                    if (ksq == keySeq.toString()) {
                        qDebug() << "emit " << ksq << " " << action->objectName();
                        emit action->triggered();
                        event->accept();
                        return;
                    }
                }
            }
        }
    }

    if (!ksq.isEmpty() && ksq != "Down" && ksq != "Up") {
        qDebug() << ksq;
    }
    QTableView::keyPressEvent(event);
}

void FileTableView::focusInEvent(QFocusEvent *event)
{
    updateMenu();

    QTableView::focusInEvent(event);
}

void FileTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);

    FileTableModel *m = static_cast<FileTableModel*>(model());
    emit indexChanged(m->absoluteFilePath(current));

    QTableView::currentChanged(current, previous);
}
