#include "common.h"
#include "deleteworker.h"
#include "filetablemodel.h"
#include "filetableview.h"
#include "mainwindow.h"
#include "operationdialog.h"
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

#define CHECK_FOCUS {                   \
        if (!hasFocus()) { return; }    \
    }

#define MENU_TRRIGGERED(x) \
    getMainWnd()->findChild<QAction*>(#x), SIGNAL(triggered())
#define MENU_TOGGLED(x) \
    getMainWnd()->findChild<QAction*>(#x), SIGNAL(toggled(bool))

FileTableView::FileTableView(QWidget *parent) :
    QTableView(parent)
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

    connect(MENU_TOGGLED(view_Hidden), this, SLOT(showHiddenFiles(bool)));
    connect(MENU_TOGGLED(view_System), this, SLOT(showSystemFiles(bool)));
    connect(MENU_TRRIGGERED(view_Sort), this, SLOT(setSort()));

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

    connect(MENU_TRRIGGERED(cmd_Delete), this, SLOT(cmdDelete()));
    connect(MENU_TRRIGGERED(cmd_Rename), this, SLOT(cmdRename()));
    connect(MENU_TRRIGGERED(cmd_NewFile), this, SLOT(newFile()));
    connect(MENU_TRRIGGERED(cmd_NewFolder), this, SLOT(newFolder()));

    connect(this, SIGNAL(doubleClicked(QModelIndex)),
            this, SLOT(setRootIndex(QModelIndex)));

}
QString FileTableView::side() const
{
    return m_side;
}

void FileTableView::setSide(const QString &side)
{
    m_side = side;
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
    dlg.resize(500, 100);
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
        update(index);
    }
    // 最終行でなければ、次のアイテムに移動する
    if (index.row() < m->rowCount() - 1) {
        setCurrentIndex(m->index(index.row() + 1, 1));
    }
}

void FileTableView::checkAllItems()
{
    CHECK_FOCUS;

    FileTableModel *m = static_cast<FileTableModel*>(model());
    setUpdatesEnabled(false);
    m->setCheckStateAll(Qt::Checked);
    setUpdatesEnabled(true);
}

void FileTableView::checkAllFiles()
{
    CHECK_FOCUS;

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
}

void FileTableView::uncheckAllItems()
{
    CHECK_FOCUS;

    FileTableModel *m = static_cast<FileTableModel*>(model());
    setUpdatesEnabled(false);
    m->setCheckStateAll(Qt::Unchecked);
    setUpdatesEnabled(true);
}

void FileTableView::invertAllChecked()
{
    CHECK_FOCUS;

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
    m->refresh();
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
    m->refresh();
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
        m->refresh();
        setUpdatesEnabled(true);
    }
}

void FileTableView::back()
{
    CHECK_FOCUS;
}

void FileTableView::forward()
{
    CHECK_FOCUS;
}

void FileTableView::showHidtory()
{
    CHECK_FOCUS;
}

void FileTableView::jumpToHome()
{
    CHECK_FOCUS;
    FileTableModel *m = static_cast<FileTableModel*>(model());

    setUpdatesEnabled(false);
    m->setPath(QDir::homePath());
    setUpdatesEnabled(true);
}

void FileTableView::jumpToParent()
{
    CHECK_FOCUS;

    FileTableModel *m = static_cast<FileTableModel*>(model());
    QDir dir(m->absolutePath());
    dir.cdUp();

    setUpdatesEnabled(false);
    m->setPath(dir.absolutePath());
    setUpdatesEnabled(true);
}

void FileTableView::jumpToRoot()
{
    CHECK_FOCUS;
    FileTableModel *m = static_cast<FileTableModel*>(model());

    setUpdatesEnabled(false);
    m->setPath(QDir::rootPath());
    setUpdatesEnabled(true);
}

void FileTableView::jumpTo()
{
    CHECK_FOCUS;

    FileTableModel *m = static_cast<FileTableModel*>(model());
    QString path = QFileDialog::getExistingDirectory(
                this, tr("フォルダを選択"), m->absolutePath());
    if (!path.isEmpty()) {
        setUpdatesEnabled(false);
        m->setPath(path);
        setUpdatesEnabled(true);
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
        DeleteWorker worker;
        worker.setDeleteList(&list);

        OperationDialog opDlg(this);
        opDlg.setWindowTitle(tr("削除"));
        opDlg.setWorker(&worker);
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
        RenameWorker worker;
        worker.setRenameMap(&dlg->renameMap());

        OperationDialog opDlg(this);
        opDlg.setWindowTitle(tr("名前を変更"));
        opDlg.setWorker(&worker);
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

void FileTableView::setRootIndex(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    FileTableModel *m = static_cast<FileTableModel*>(model());

    if (m->isDir(index)) {
        setUpdatesEnabled(false);
        m->setPath(m->absoluteFilePath(index));
        selectRow(0);
        setUpdatesEnabled(true);
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
                if (ksq == action->shortcut().toString()) {
                    qDebug() << "emit " << ksq << " " << action->objectName();
                    emit action->triggered();
                    event->accept();
                    return;
                }
            }
        }
    }

    qDebug() << ksq;
    QTableView::keyPressEvent(event);

}
