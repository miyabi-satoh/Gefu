#include "copyworker.h"
#include "deleteworker.h"
#include "mainwindow.h"
#include "operationdialog.h"
#include "overwritedialog.h"
#include "renamemultidialog.h"
#include "renamesingledialog.h"
#include "renameworker.h"
#include "ui_mainwindow.h"
#include <QFileSystemModel>
#include <QDebug>
#include <QKeyEvent>
#include <QDesktopServices>
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QThread>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings;
    bool checked = settings.value("Common/ShowHidden", false).toBool();
    ui->view_Hidden->setChecked(checked);

    // メニューのシグナル/スロットを設定する
    connect(ui->action_Command, SIGNAL(triggered()), this, SLOT(onActionCommand()));
    connect(ui->action_Exec, SIGNAL(triggered()), this, SLOT(onActionExec()));
    connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(onActionOpen()));
    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(onActionQuit()));
    connect(ui->action_Setting, SIGNAL(triggered()), this, SLOT(onActionSetting()));

    connect(ui->mark_All, SIGNAL(triggered()), this, SLOT(onMarkAll()));
    connect(ui->mark_AllFiles, SIGNAL(triggered()), this, SLOT(onMarkAllFiles()));
    connect(ui->mark_AllOff, SIGNAL(triggered()), this, SLOT(onMarkAllOff()));
    connect(ui->mark_Invert, SIGNAL(triggered()), this, SLOT(onMarkInvert()));
    connect(ui->mark_Toggle, SIGNAL(triggered()), this, SLOT(onMarkToggle()));

    connect(ui->move_Down, SIGNAL(triggered()), this, SLOT(onMoveCursorDown()));
    connect(ui->move_Up, SIGNAL(triggered()), this, SLOT(onMoveCursorUp()));
    connect(ui->move_Begin, SIGNAL(triggered()), this, SLOT(onMoveCursorBegin()));
    connect(ui->move_End, SIGNAL(triggered()), this, SLOT(onMoveCursorEnd()));
    connect(ui->move_Back, SIGNAL(triggered()), this, SLOT(onMoveBack()));
    connect(ui->move_Forward, SIGNAL(triggered()), this, SLOT(onMoveForward()));
    connect(ui->move_Home, SIGNAL(triggered()), this, SLOT(onMoveHome()));
    connect(ui->move_Jump, SIGNAL(triggered()), this, SLOT(onMoveJump()));
    connect(ui->move_Parent, SIGNAL(triggered()), this, SLOT(onMoveParent()));
    connect(ui->move_Root, SIGNAL(triggered()), this, SLOT(onMoveRoot()));

    connect(ui->view_FromOther, SIGNAL(triggered()), this, SLOT(onViewFromOther()));
    connect(ui->view_ToOther, SIGNAL(triggered()), this, SLOT(onViewToOther()));
    connect(ui->view_Hidden, SIGNAL(triggered()), this, SLOT(onViewHidden()));
    connect(ui->view_Swap, SIGNAL(triggered()), this, SLOT(onViewSwap()));

    connect(ui->cmd_Copy, SIGNAL(triggered()), this, SLOT(onCmdCopy()));
    connect(ui->cmd_Delete, SIGNAL(triggered()), this, SLOT(onCmdDelete()));
    connect(ui->cmd_NewFile, SIGNAL(triggered()), this, SLOT(onCmdNewFile()));
    connect(ui->cmd_NewFolder, SIGNAL(triggered()), this, SLOT(onCmdNewFolder()));
    connect(ui->cmd_Rename, SIGNAL(triggered()), this, SLOT(onCmdRename()));

    connect(ui->help_About, SIGNAL(triggered()), this, SLOT(onHelpAbout()));

    // ウィンドウタイトルを設定する
    setWindowTitle(tr("げふぅ v0.00"));
    // ウィンドウアイコンを設定する
    setWindowIcon(QIcon(":/images/Gefu.png"));

    // ウィンドウ初期サイズを設定する
    resize(800, 600);

    QString path;

    path = settings.value("Left/dir", QDir::homePath()).toString();
    ui->folderPanel_L->setCurrentFolder(path);

    path = settings.value("Right/dir", QDir::homePath()).toString();
    ui->folderPanel_R->setCurrentFolder(path);
}

MainWindow::~MainWindow()
{
    QSettings settings;

    settings.setValue("Left/dir", ui->folderPanel_L->dir()->absolutePath());
    settings.setValue("Right/dir", ui->folderPanel_R->dir()->absolutePath());

    delete ui;
}

void MainWindow::setStatusText(const QString &str)
{
    ui->statusBar->showMessage(str);
}

FolderPanel* MainWindow::activePanel()
{
    if (ui->folderPanel_L->fileTable()->hasFocus()) {
        return ui->folderPanel_L;
    }
    if (ui->folderPanel_R->fileTable()->hasFocus()) {
        return ui->folderPanel_R;
    }

    return NULL;
}

FolderPanel* MainWindow::inactivePanel()
{
    FolderPanel *fp = activePanel();
    if (fp == ui->folderPanel_L) {
        return ui->folderPanel_R;
    }
    if (fp == ui->folderPanel_R) {
        return ui->folderPanel_L;
    }

    return NULL;
}

///
/// \brief MainWindow::onActionCommand
///
/// コマンドを実行します(X)
///
void MainWindow::onActionCommand()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    QString command = "";
    for (int n = 0; n < fp->fileTable()->rowCount(); n++) {
        if (fp->fileTable()->item(n, 0)->checkState() == Qt::Checked) {
            QString path = fp->fileTable()->item(n, 1)->text();
            path = fp->dir()->absoluteFilePath(path);
            path = QDir::cleanPath(path);
            path = QDir::toNativeSeparators(path);

            QFileInfo info(path);
            if (info.isExecutable()) {
                command = "\"" + path + "\" " + command;
            }
            else {
                command += " \"" + path + "\"";
            }
        }
    }

    if (command.isEmpty()) {
//        int row = fp->fileTable()->currentIndex().row();
        int row = fp->fileTable()->currentRow();
        QString path = fp->fileTable()->item(row, 1)->text();
        path = fp->dir()->absoluteFilePath(path);
        path = QDir::cleanPath(path);
        path = QDir::toNativeSeparators(path);

        command = "\"" + path + "\"";
    }

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setLabelText(tr("コマンドを入力："));
    dlg.setWindowTitle(tr("コマンドを実行"));
    dlg.setTextValue(command);
    dlg.resize(500, 100);
    int ret = dlg.exec();
    command = dlg.textValue();
    if (ret == QDialog::Accepted && !command.isEmpty()) {
        QProcess process(this);
        process.setWorkingDirectory(fp->dir()->absolutePath());
        if (!process.startDetached(command)) {
            QMessageBox::critical(
                        this,
                        tr("エラー"),
                        tr("コマンドの実行に失敗しました。<br/>") + command);
        }
    }
}

///
/// \brief MainWindow::onActionExec
///
/// アプリケーションで開きます(Shift + Enter)
///
void MainWindow::onActionExec()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    int count = 0;
    for (int n = 0; n < fp->fileTable()->rowCount(); n++) {
        if (fp->fileTable()->item(n, 0)->checkState() == Qt::Checked) {
            QString path = fp->fileTable()->item(n, 1)->text();
            path = fp->dir()->absoluteFilePath(path);
            path = QDir::toNativeSeparators(path);
            QDesktopServices::openUrl(QUrl("file:///" + path));
            count++;
        }
    }

    if (count == 0) {
//        int row = fp->fileTable()->currentIndex().row();
        int row = fp->fileTable()->currentRow();
        QString path = fp->fileTable()->item(row, 1)->text();
        path = fp->dir()->absoluteFilePath(path);
        path = QDir::toNativeSeparators(path);
        QDesktopServices::openUrl(QUrl("file:///" + path));
    }
}

///
/// \brief MainWindow::onActionOpen
///
/// フォルダを開きます(Enter)
///
void MainWindow::onActionOpen()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

//    int row = fp->fileTable()->currentIndex().row();
    int row = fp->fileTable()->currentRow();
    QString path = fp->fileTable()->item(row, 1)->text();
    path = fp->dir()->absoluteFilePath(path);
    QFileInfo info(path);

    if (info.isDir()) {
        fp->setCurrentFolder(path);
    }
}

///
/// \brief MainWindow::onActionQuit
///
/// アプリケーションを終了します(Q)
///
void MainWindow::onActionQuit()
{
    qApp->quit();
}

///
/// \brief MainWindow::onActionSetting
///
/// 環境設定ダイアログを表示します(Z)
///
void MainWindow::onActionSetting()
{
    QMessageBox::information(this, tr("情報"), tr("環境設定機能は未実装です。"));
}

///
/// \brief MainWindow::on_mark_All_triggered
///
/// すべてマークします(Shift + A)
///
void MainWindow::onMarkAll()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        fp->fileTable()->item(row, 0)->setCheckState(Qt::Checked);
    }
}

///
/// \brief MainWindow::onMarkAllFiles
///
/// すべての「ファイル」をマークします(A)
///
void MainWindow::onMarkAllFiles()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        QString path = fp->fileTable()->item(row, 1)->text();
        path = fp->dir()->absoluteFilePath(path);

        QFileInfo info(path);
        if (info.isDir()) {
            fp->fileTable()->item(row, 0)->setCheckState(Qt::Unchecked);
        }
        else {
           fp->fileTable()->item(row, 0)->setCheckState(Qt::Checked);
        }
    }
}

///
/// \brief MainWindow::onMarkAllOff
///
/// すべてのマークを解除します(U)
///
void MainWindow::onMarkAllOff()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        fp->fileTable()->item(row, 0)->setCheckState(Qt::Unchecked);
    }
}

///
/// \brief MainWindow::onMarkInvert
///
/// マークを反転します(I)
///
void MainWindow::onMarkInvert()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        QTableWidgetItem *item = fp->fileTable()->item(row, 0);
        if (item->checkState() == Qt::Checked) {
            item->setCheckState(Qt::Unchecked);
        }
        else {
            item->setCheckState(Qt::Checked);
        }
    }
}

///
/// \brief MainWindow::onMarkToggle
///
/// マークを設定または解除します(Space)
///
void MainWindow::onMarkToggle()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

//    int row = fp->fileTable()->currentIndex().row();
    int row = fp->fileTable()->currentRow();
    QTableWidgetItem *item = fp->fileTable()->item(row, 0);
    if (fp->fileTable()->item(row, 1)->text() != "..") {
       if (item->checkState() == Qt::Checked) {
            item->setCheckState(Qt::Unchecked);
       }
        else {
            item->setCheckState(Qt::Checked);
       }
    }
    // 最終行でなければ、次のアイテムに移動する
    if (row < fp->fileTable()->rowCount() - 1) {
        QModelIndex nextIndex = fp->fileTable()->model()->index(row + 1, 1);
        fp->fileTable()->setCurrentIndex(nextIndex);
    }
}

///
/// \brief MainWindow::onMoveCursorDown
///
/// カーソルを下に移動します(J)
///
void MainWindow::onMoveCursorDown()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

//    int row = fp->fileTable()->currentIndex().row();
    int row = fp->fileTable()->currentRow();
    if (row < fp->fileTable()->rowCount() - 1) {
        QModelIndex nextIndex = fp->fileTable()->model()->index(row + 1, 1);
        fp->fileTable()->setCurrentIndex(nextIndex);
    }
}

///
/// \brief MainWindow::onMoveCursorUp
///
/// カーソルを上に移動します(K)
///
void MainWindow::onMoveCursorUp()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

//    int row = fp->fileTable()->currentIndex().row();
    int row = fp->fileTable()->currentRow();
    if (row > 0) {
        QModelIndex nextIndex = fp->fileTable()->model()->index(row - 1, 1);
        fp->fileTable()->setCurrentIndex(nextIndex);
    }
}

///
/// \brief MainWindow::onMoveCursorBegin
///
/// カーソルを先頭に移動します(G)
///
void MainWindow::onMoveCursorBegin()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    QModelIndex nextIndex = fp->fileTable()->model()->index(0, 1);
    fp->fileTable()->setCurrentIndex(nextIndex);
}

///
/// \brief MainWindow::onMoveCursorEnd
///
/// カーソルを末尾に移動します(Shift + G)
void MainWindow::onMoveCursorEnd()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = fp->fileTable()->rowCount() - 1;
    QModelIndex nextIndex = fp->fileTable()->model()->index(row, 1);
    fp->fileTable()->setCurrentIndex(nextIndex);
}

///
/// \brief MainWindow::onMoveBack
///
/// 前の履歴に戻ります(Alt + ←)
///
void MainWindow::onMoveBack()
{
    QMessageBox::information(this, tr("情報"), tr("履歴機能は未実装です。"));
}

///
/// \brief MainWindow::onMoveForward
///
/// 次の履歴に進みます(Alt + →)
///
void MainWindow::onMoveForward()
{
    QMessageBox::information(this, tr("情報"), tr("履歴機能は未実装です。"));

}

///
/// \brief MainWindow::onMoveHome
///
/// ホームフォルダに移動します(H)
///
void MainWindow::onMoveHome()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    fp->setCurrentFolder(QDir::homePath());
}

///
/// \brief MainWindow::onMoveJump
///
/// フォルダを選択して移動します(Shift + J)
void MainWindow::onMoveJump()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    QString path = QFileDialog::getExistingDirectory(
                this,
                tr("フォルダを選択"),
                fp->dir()->absolutePath());
    if (!path.isEmpty()) {
        fp->setCurrentFolder(path);
    }
}

///
/// \brief MainWindow::onMoveOther
///
/// キーボードフォーカスを他方のパネルに移動します(TAB)
///
void MainWindow::onMoveOther()
{
    FolderPanel *fp = this->inactivePanel();
    if (fp) {
        fp->fileTable()->setFocus();
    }
}

///
/// \brief MainWindow::onMoveParent
///
/// 親フォルダに移動します(Backspace)
///
void MainWindow::onMoveParent()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    if (!fp->dir()->isRoot()) {
        QString path = fp->dir()->absoluteFilePath("..");
        fp->setCurrentFolder(path);
    }
}

///
/// \brief MainWindow::onMoveRoot
///
/// ルートフォルダに移動します()
void MainWindow::onMoveRoot()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    if (!fp->dir()->isRoot()) {
        fp->setCurrentFolder(QDir::rootPath());
    }
}

///
/// \brief MainWindow::onViewFromOther
///
/// 隣のパネルと同じフォルダを表示します(O)
///
void MainWindow::onViewFromOther()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    fp1->setCurrentFolder(fp2->dir()->absolutePath());
}

///
/// \brief MainWindow::onViewToOther
///
/// 隣のパネルに同じフォルダを表示します(Shift + O)
///
void MainWindow::onViewToOther()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    fp2->setCurrentFolder(fp1->dir()->absolutePath());
}

///
/// \brief MainWindow::onViewHidden
///
/// 隠しファイルの表示/非表示を切り替えます(Shift + H)
void MainWindow::onViewHidden()
{
    QSettings settings;
    bool checked = !settings.value("Common/ShowHidden", false).toBool();
    settings.setValue("Common/ShowHidden", checked);

    if (checked) {
        ui->folderPanel_L->dir()->setFilter(ui->folderPanel_L->dir()->filter() | QDir::Hidden);
        ui->folderPanel_R->dir()->setFilter(ui->folderPanel_R->dir()->filter() | QDir::Hidden);
    }
    else {
        ui->folderPanel_L->dir()->setFilter(ui->folderPanel_L->dir()->filter() ^ QDir::Hidden);
        ui->folderPanel_R->dir()->setFilter(ui->folderPanel_R->dir()->filter() ^ QDir::Hidden);
    }

    ui->folderPanel_L->setCurrentFolder(ui->folderPanel_L->dir()->absolutePath());
    ui->folderPanel_R->setCurrentFolder(ui->folderPanel_R->dir()->absolutePath());
}

///
/// \brief MainWindow::onViewSwap
///
/// パネルの表示内容を交換します(W)
///
void MainWindow::onViewSwap()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    QString path1 = fp1->dir()->absolutePath();
    QString path2 = fp2->dir()->absolutePath();

    fp1->setCurrentFolder(path2);
    fp2->setCurrentFolder(path1);
}

void MainWindow::onCmdCopy()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    QStringList list = selectedItems(fp);
    if (list.isEmpty()) {
        return;
    }

    CopyWorker *worker = new CopyWorker();
    connect(worker, SIGNAL(askOverWrite(bool*,int*,int*,QString*,QString,QString)),
            this, SLOT(onAskOverWrite(bool*,int*,int*,QString*,QString,QString)));
    worker->setCopyList(&list);
    worker->setTargetDir(inactivePanel()->dir()->absolutePath());

    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("コピー"));
    opDlg.setWorker(worker);

    ui->folderPanel_L->UninstallWatcher();
    ui->folderPanel_R->UninstallWatcher();
    opDlg.exec();
    ui->folderPanel_L->setCurrentFolder(ui->folderPanel_L->dir()->absolutePath());
    ui->folderPanel_R->setCurrentFolder(ui->folderPanel_R->dir()->absolutePath());

}

void MainWindow::onAskOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod,
                                QString *alias, const QString srcPath,
                                const QString tgtPath)
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
    CopyWorker *worker = static_cast<CopyWorker*>(sender());
    worker->endAsking();
}

///
/// \brief MainWindow::onCmdDelete
///
/// ファイルを削除します
///
void MainWindow::onCmdDelete()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    QStringList list = selectedItems(fp);
    if (list.isEmpty()) {
        return;
    }

    QString msg;
    if (list.size() == 1) {
        msg = QFileInfo(list.at(0)).fileName();
    }
    else {
        msg = tr("%1個のアイテム").arg(list.size());
    }
    int ret = QMessageBox::question(
                this,
                tr("確認"),
                msg + tr("を削除します<br/>よろしいですか？"));
    if (ret == QMessageBox::Yes) {
        DeleteWorker *worker = new DeleteWorker();
        worker->setDeleteList(&list);

        OperationDialog opDlg(this);
        opDlg.setWindowTitle(tr("削除"));
        opDlg.setWorker(worker);

        ui->folderPanel_L->UninstallWatcher();
        ui->folderPanel_R->UninstallWatcher();
        opDlg.exec();
        ui->folderPanel_L->setCurrentFolder(ui->folderPanel_L->dir()->absolutePath());
        ui->folderPanel_R->setCurrentFolder(ui->folderPanel_R->dir()->absolutePath());
    }
}

///
/// \brief MainWindow::onCmdNewFile
///
/// ファイルを作成します
///
void MainWindow::onCmdNewFile()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    bool bOk;
    QString name = QInputDialog::getText(
                this,
                tr("ファイルを作成"),
                tr("ファイル名"),
                QLineEdit::Normal,
                "",
                &bOk);
    if (bOk && !name.isEmpty()) {
        QFile file(fp->dir()->absoluteFilePath(name));
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this,
                                  tr("エラー"),
                                  tr("ファイルの作成に失敗しました。"));
        }
        else {
            file.close();
        }
    }
}

///
/// \brief MainWindow::onCmdNewFolder
///
/// フォルダを作成します
///
void MainWindow::onCmdNewFolder()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    bool bOk;
    QString name = QInputDialog::getText(
                this,
                tr("フォルダを作成"),
                tr("フォルダ名"),
                QLineEdit::Normal,
                "",
                &bOk);
    if (bOk && !name.isEmpty()) {
        if (!fp->dir()->mkpath(name)) {
            QMessageBox::critical(this,
                                  tr("エラー"),
                                  tr("フォルダの作成に失敗しました。"));
        }
    }
}

///
/// \brief MainWindow::onCmdRename
///
/// 名前を変更します
///
void MainWindow::onCmdRename()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    QStringList list = selectedItems(fp);
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
    dlg->setWorkingDirectory(fp->dir()->absolutePath());
    dlg->setNames(list);
    int dlgResult = dlg->exec();
    if (dlgResult == QDialog::Accepted && !dlg->renameMap().isEmpty()) {
        RenameWorker *worker = new RenameWorker();
        worker->setRenameMap(&dlg->renameMap());

        OperationDialog opDlg(this);
        opDlg.setWindowTitle(tr("名前を変更"));
        opDlg.setWorker(worker);

        ui->folderPanel_L->UninstallWatcher();
        ui->folderPanel_R->UninstallWatcher();
        opDlg.exec();
        ui->folderPanel_L->setCurrentFolder(ui->folderPanel_L->dir()->absolutePath());
        ui->folderPanel_R->setCurrentFolder(ui->folderPanel_R->dir()->absolutePath());
    }
}

///
/// \brief MainWindow::onHelpAbout
///
/// アプリケーションの概要を表示します(?)
///
void MainWindow::onHelpAbout()
{
    QMessageBox::about(
                this,
                tr("げふぅ について"),
                tr("<h3>Gefu Ver0.00</h3>"
                   "<center>Gefu is Experimental File Utility.<br/>"
                   "（げふぅは実験的なファイルユーティリティです）</center>"
                   "<p>Copyright 2014 @miyabi_satoh All rights reserved.</p>"));
}

QStringList MainWindow::selectedItems(FolderPanel *fp)
{
    QStringList list;
    for (int n = 0; n < fp->fileTable()->rowCount(); n++) {
        if (fp->fileTable()->item(n, 0)->checkState() == Qt::Checked) {
            list << fp->dir()->absoluteFilePath(fp->fileTable()->item(n, 1)->text());
        }
    }

    if (list.isEmpty()) {
        int row = fp->fileTable()->currentRow();
        QString name = fp->fileTable()->item(row, 1)->text();
        if (name != "..") {
            list << fp->dir()->absoluteFilePath(name);
        }
    }

    return list;
}
