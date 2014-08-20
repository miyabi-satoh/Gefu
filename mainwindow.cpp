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
    ui->RPanel->setSide("Right");
    ui->LPanel->setSide("Left");
    ui->LPanel->fileTable()->setFocus();

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
    connect(ui->view_Hidden, SIGNAL(triggered()), this, SLOT(toggleShowHiddenFiles()));
    connect(ui->view_System, SIGNAL(triggered()), this, SLOT(toggleShowSystemFiles()));

    // ウィンドウタイトルを設定する
    setWindowTitle(tr("げふぅ v%1").arg(VERSION_VALUE));
    // ウィンドウアイコンを設定する
    setWindowIcon(QIcon(":/images/Gefu.png"));

    // ウィンドウ初期サイズを設定する
    resize(800, 600);
}

MainWindow::~MainWindow()
{

    delete ui;
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

void MainWindow::onActionSetting()
{
    QMessageBox::information(this, tr("情報"), tr("環境設定機能は未実装です。"));
}

void MainWindow::toggleShowHiddenFiles()
{
    // キーボードがトリガーの場合、メニュー項目のチェック状態は
    // 変わらないので、QSettingsを使う
    QSettings settings;
    bool show = !settings.value(IniKey_ShowHidden, false).toBool();
    settings.setValue(IniKey_ShowHidden, show);

    ui->view_Hidden->setChecked(show);

    emit showHiddenFiles(show);
}

void MainWindow::toggleShowSystemFiles()
{
    // キーボードがトリガーの場合、メニュー項目のチェック状態は
    // 変わらないので、QSettingsを使う
    QSettings settings;
    bool show = !settings.value(IniKey_ShowSystem, false).toBool();
    settings.setValue(IniKey_ShowSystem, show);

    ui->view_System->setChecked(show);

    emit showSystemFiles(show);
}

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
