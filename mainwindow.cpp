#include "copymoveworker.h"
#include "deleteworker.h"
#include "mainwindow.h"
#include "operationdialog.h"
#include "overwritedialog.h"
#include "renamemultidialog.h"
#include "renamesingledialog.h"
#include "renameworker.h"
#include "sortdialog.h"
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
    ui->LPanel->setSide("Left");
    ui->RPanel->setSide("Right");

    QSettings settings;
    // メニュー項目のチェック状態を初期化する
    if (settings.value(IniKey_ShowHidden, false).toBool()) {
        ui->view_Hidden->setChecked(true);
    }
    if (settings.value(IniKey_ShowSystem, false).toBool()) {
        ui->view_System->setChecked(true);
    }
    // 追加のショートカットキーを設定する
    QList<QKeySequence> shortcuts;
    shortcuts = ui->action_Open->shortcuts();
    shortcuts.append(QKeySequence("M"));
    ui->action_Open->setShortcuts(shortcuts);

    shortcuts = ui->action_Exec->shortcuts();
    shortcuts.append(QKeySequence("Shift+M"));
    ui->action_Exec->setShortcuts(shortcuts);

    // シグナル/スロットを設定する
    connect(ui->action_Setting, SIGNAL(triggered()), this, SLOT(onActionSetting()));
    connect(ui->action_Quit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(ui->help_About, SIGNAL(triggered()), this, SLOT(onHelpAbout()));


//    connect(ui->view_FromOther, SIGNAL(triggered()), this, SLOT(onViewFromOther()));
//    connect(ui->view_ToOther, SIGNAL(triggered()), this, SLOT(onViewToOther()));
//    connect(ui->view_Swap, SIGNAL(triggered()), this, SLOT(onViewSwap()));

//    connect(ui->cmd_Copy, SIGNAL(triggered()), this, SLOT(onCmdCopy()));
//    connect(ui->cmd_Move, SIGNAL(triggered()), this, SLOT(onCmdMove()));


    // ウィンドウタイトルを設定する
    setWindowTitle(tr("げふぅ v%1").arg(VERSION_VALUE));
    // ウィンドウアイコンを設定する
    setWindowIcon(QIcon(":/images/Gefu.png"));

    // ウィンドウ初期サイズを設定する
    resize(800, 600);

}

MainWindow::~MainWindow()
{
    QSettings settings;

    settings.setValue(IniKey_LeftDir, ui->LPanel->dir()->absolutePath());
    settings.setValue(IniKey_RightDir, ui->RPanel->dir()->absolutePath());

    delete ui;
}

void MainWindow::setStatusText(const QString &str)
{
    ui->statusBar->showMessage(str);
}

FileTableView *MainWindow::otherSideView(const FileTableView *view) const
{
    QWidget *w = view->parentWidget();
    if (ui->LPanel == w) {
        return ui->RPanel->findChild<FileTableView*>("fileTable");
    }
    else if (ui->RPanel == w) {
        return ui->LPanel->findChild<FileTableView*>("fileTable");
    }

    qDebug() << w->objectName();
    return NULL;
}

FolderPanel* MainWindow::activePanel()
{
    if (ui->LPanel->fileTable()->hasFocus()) {
        return ui->LPanel;
    }
    if (ui->RPanel->fileTable()->hasFocus()) {
        return ui->RPanel;
    }

    return NULL;
}

FolderPanel* MainWindow::inactivePanel()
{
    FolderPanel *fp = activePanel();
    if (fp == ui->LPanel) {
        return ui->RPanel;
    }
    if (fp == ui->RPanel) {
        return ui->LPanel;
    }

    return NULL;
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


///
/// \brief MainWindow::onCmdMove
///
/// ファイルを移動します(Ctrl + M)
///
void MainWindow::onCmdMove()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

//    QStringList list = selectedItems(fp);
//    if (list.isEmpty()) {
//        return;
//    }

//    CopyMoveWorker *worker = new CopyMoveWorker();
//    connect(worker, SIGNAL(askOverWrite(bool*,int*,int*,QString*,QString,QString)),
//            this, SLOT(onAskOverWrite(bool*,int*,int*,QString*,QString,QString)));
//    worker->setCopyList(&list);
//    worker->setTargetDir(inactivePanel()->dir()->absolutePath());
//    worker->setMoveMode(true);

//    OperationDialog opDlg(this);
//    opDlg.setWindowTitle(tr("移動"));
//    opDlg.setWorker(worker);

//    ui->LPanel->UninstallWatcher();
//    ui->RPanel->UninstallWatcher();
//    opDlg.exec();
//    ui->LPanel->setCurrentFolder(ui->LPanel->dir()->absolutePath());
//    ui->RPanel->setCurrentFolder(ui->RPanel->dir()->absolutePath());

}

///
/// \brief MainWindow::onCmdCopy
///
/// ファイルをコピーします(Ctrl + C)
///
void MainWindow::onCmdCopy()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

//    QStringList list = selectedItems(fp);
//    if (list.isEmpty()) {
//        return;
//    }

//    CopyMoveWorker *worker = new CopyMoveWorker();
//    connect(worker, SIGNAL(askOverWrite(bool*,int*,int*,QString*,QString,QString)),
//            this, SLOT(onAskOverWrite(bool*,int*,int*,QString*,QString,QString)));
//    worker->setCopyList(&list);
//    worker->setTargetDir(inactivePanel()->dir()->absolutePath());
//    worker->setMoveMode(false);

//    OperationDialog opDlg(this);
//    opDlg.setWindowTitle(tr("コピー"));
//    opDlg.setWorker(worker);

//    ui->LPanel->UninstallWatcher();
//    ui->RPanel->UninstallWatcher();
//    opDlg.exec();
//    ui->LPanel->setCurrentFolder(ui->LPanel->dir()->absolutePath());
//    ui->RPanel->setCurrentFolder(ui->RPanel->dir()->absolutePath());

}

///
/// \brief MainWindow::onAskOverWrite
/// \param bOk
/// \param prevCopyMethod
/// \param copyMethod
/// \param alias
/// \param srcPath
/// \param tgtPath
///
/// 上書き処理の方法をユーザに問い合わせます
///
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
    CopyMoveWorker *worker = static_cast<CopyMoveWorker*>(sender());
    worker->endAsking();
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
                tr("<h3>Gefu Ver%1</h3>").arg(VERSION_VALUE) +
                tr("<center>Gefu is Experimental File Utility.<br/>"
                   "（げふぅは実験的なファイルユーティリティです）</center>"
                   "<p>Copyright 2014 @miyabi_satoh All rights reserved.</p>"));
}

///
/// \brief getMainWnd
/// \return メインウィンドウのポインタ
///
/// メインウィンドウを取得します
///
MainWindow* getMainWnd()
{
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if (w->objectName() == "MainWindow") {
            return static_cast<MainWindow*>(w);
        }
    }
    qDebug() << "MainWindow not found !?";
    return NULL;
}
