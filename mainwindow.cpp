#include "common.h"
#include "mainwindow.h"
#include "preferencedialog.h"
#include "filetableview.h"
#include "ui_mainwindow.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->RPanel->setSide("Right");
    ui->LPanel->setSide("Left");
    ui->LPanel->updateAppearance();
    ui->RPanel->updateAppearance();

    QSettings settings;
    // メニュー項目の状態を初期化する
    if (settings.value(IniKey_ShowHidden).toBool()) {
        ui->view_Hidden->setChecked(true);
    }
    if (settings.value(IniKey_ShowSystem).toBool()) {
        ui->view_System->setChecked(true);
    }
    bool enabled;
    enabled = !settings.value(IniKey_EditorPath).toString().isEmpty();
    ui->action_OpenEditor->setEnabled(enabled);

    enabled = !settings.value(IniKey_TerminalPath).toString().isEmpty();
    ui->action_OpenTerminal->setEnabled(enabled);

    // 追加のショートカットキーを設定する
    QList<QKeySequence> shortcuts;
    shortcuts = ui->action_Open->shortcuts();
    shortcuts.append(QKeySequence("M"));
    ui->action_Open->setShortcuts(shortcuts);

    shortcuts = ui->action_Exec->shortcuts();
    shortcuts.append(QKeySequence("Shift+M"));
    ui->action_Exec->setShortcuts(shortcuts);

    shortcuts = ui->view_Filter->shortcuts();
    shortcuts.append(QKeySequence("Shift+*"));  // マヂで！？
    ui->view_Filter->setShortcuts(shortcuts);

    shortcuts = ui->action_OpenTerminal->shortcuts();
    shortcuts.append(QKeySequence("Shift+>"));  // マヂで！？
    ui->action_OpenTerminal->setShortcuts(shortcuts);

    shortcuts = ui->help_About->shortcuts();
    shortcuts.append(QKeySequence("Shift+?"));  // マヂで！？
    ui->help_About->setShortcuts(shortcuts);

    // シグナル/スロットを設定する
    connect(ui->action_Setting, SIGNAL(triggered()), this, SLOT(onActionSetting()));
    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->help_About, SIGNAL(triggered()), this, SLOT(onHelpAbout()));
    connect(ui->view_Hidden, SIGNAL(triggered()), this, SLOT(toggleShowHiddenFiles()));
    connect(ui->view_System, SIGNAL(triggered()), this, SLOT(toggleShowSystemFiles()));
    connect(ui->check_Update, SIGNAL(triggered()), this, SLOT(checkUpdate()));

    // ウィンドウタイトルを設定する
    setWindowTitle(tr("げふぅ v%1").arg(VERSION_VALUE));
    // ウィンドウアイコンを設定する
#if defined(Q_OS_MAC) || defined(Q_OS_MAC64) || defined(Q_OS_MACX)
    setWindowIcon(QIcon(":/images/Gefu.icns"));
#else
    setWindowIcon(QIcon(":/images/Gefu.png"));
#endif
    //>>>>> ウィンドウサイズと位置を設定する
    QString strValue;
    QPoint point = this->geometry().topLeft();
    QSize size = this->geometry().size();
    //>>>> 前回の位置・サイズ・状態を復元する
    if (!settings.value(IniKey_WindowGeometry, "").toString().isEmpty()) {
        restoreGeometry(settings.value(IniKey_WindowGeometry).toByteArray());
        restoreState(settings.value(iniKey_WindowState).toByteArray());
    }
    //>>>> INIファイルの設定から復元する
    //>>> サイズ
    strValue = settings.value(IniKey_BootSizeSpec).toString();
    if (strValue == "sizeAbsolute") {
        size = settings.value(IniKey_BootSizeAbs).toSize();
    }
    else if (strValue == "sizeRelative") {
        size = settings.value(IniKey_BootSizeRel).toSize();
        size.setWidth(size.width() * QApplication::desktop()->width() / 100.0);
        size.setHeight(size.height() * QApplication::desktop()->height() / 100.0);
    }
    else if (strValue == "sizeLast") {
        size = this->geometry().size();
    }
    //>>> 位置
    strValue = settings.value(IniKey_BootPosSpec).toString();
    if (strValue == "posAbsolute") {
        point = settings.value(IniKey_BootPosAbs).toPoint();
    }
    else if (strValue == "posRelative") {
        point = settings.value(IniKey_BootPosRel).toPoint();
        point.setX(point.x() * QApplication::desktop()->width() / 100.0);
        point.setY(point.y() * QApplication::desktop()->height() / 100.0);
    }
    else if (strValue == "posLast") {
        point = this->geometry().topLeft();
    }
    else {
        point.setX((QApplication::desktop()->width() - size.width()) / 2);
        point.setY((QApplication::desktop()->height() - size.height()) / 2);
    }
    this->setGeometry(QRect(point, size));

    // 最新バージョンをチェックする
    if (settings.value(IniKey_CheckUpdates).toBool()) {
        checkUpdate(true);
    }
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
    PreferenceDialog dlg(this);

    if (dlg.exec() == QDialog::Accepted) {
        ui->LPanel->updateAppearance();
        ui->RPanel->updateAppearance();

        QSettings settings;
        bool enabled;
        enabled = !settings.value(IniKey_EditorPath).toString().isEmpty();
        ui->action_OpenEditor->setEnabled(enabled);

        enabled = !settings.value(IniKey_TerminalPath).toString().isEmpty();
        ui->action_OpenTerminal->setEnabled(enabled);
    }
}

void MainWindow::toggleShowHiddenFiles()
{
    // キーボードがトリガーの場合、メニュー項目のチェック状態は
    // 変わらない(Mac)ので、QSettingsを使う
    QSettings settings;
    bool show = !settings.value(IniKey_ShowHidden).toBool();
    settings.setValue(IniKey_ShowHidden, show);

    ui->view_Hidden->setChecked(show);

    emit showHiddenFiles(show);
}

void MainWindow::toggleShowSystemFiles()
{
    // キーボードがトリガーの場合、メニュー項目のチェック状態は
    // 変わらない(Mac)ので、QSettingsを使う
    QSettings settings;
    bool show = !settings.value(IniKey_ShowSystem).toBool();
    settings.setValue(IniKey_ShowSystem, show);

    ui->view_System->setChecked(show);

    emit showSystemFiles(show);
}

void MainWindow::checkUpdate(bool silent)
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    if (silent) {
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(checkUpdateFinishedSilent(QNetworkReply*)));
    }
    else {
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(checkUpdateFinished(QNetworkReply*)));
    }
#ifdef Q_OS_WIN
    manager->get(QNetworkRequest(QUrl("http://miyabi.rakusaba.jp/wp-content/uploads/gefu_latest_win.html")));
#elif defined(Q_OS_MAC)
    manager->get(QNetworkRequest(QUrl("http://miyabi.rakusaba.jp/wp-content/uploads/gefu_latest_mac.html")));
#endif
}

void MainWindow::checkUpdateFinished(QNetworkReply *reply)
{
    checkUpdateFinished(reply, false);
}

void MainWindow::checkUpdateFinished(QNetworkReply *reply, bool silent)
{
    if (reply->error() == QNetworkReply::NoError) {
        QString str = reply->readAll();
        str = str.split(QRegExp("[\r\n]"), QString::SkipEmptyParts).at(0);
        QRegExp rx("Gefu([0-9]+)");
        rx.indexIn(str);
        QString version = rx.cap(1);
        if (version.toInt() > VERSION_VALUE * 100) {
            QMessageBox::information(
                        this, tr("情報"),
                        tr("最新のバージョンが存在します。<br/>") +
                        tr("<a href='%1'>こちらからダウンロードしてください。</a>").arg(str));
        }
        else if (!silent) {
            QMessageBox::information(
                        this, tr("情報"),
                        tr("お使いのバージョンは最新です。"));
        }
    }
    else if (!silent){
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("最新バージョンのチェックに失敗しました。<br/>") +
                    reply->errorString());
    }
}

void MainWindow::checkUpdateFinishedSilent(QNetworkReply *reply)
{
    checkUpdateFinished(reply, true);
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(
                this,
                tr("げふぅ について"),
                tr("<h3>Gefu Ver%1</h3>").arg(VERSION_VALUE) +
                tr("<center>Gefu is an Experimental File Utility.<br/>"
                   "<small>（げふぅは実験的なファイルユーティリティです）</small></center>"
                   "<p>最新版の情報は<a href='http://miyabi.rakusaba.jp'>喫茶[雅]</a>で公開しています。</p>"
                   "<p><small>Copyright 2014 @miyabi_satoh All rights reserved.</small></p>"));
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


void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;

    if (settings.value(IniKey_ConfirmExit).toBool()) {
        QMessageBox msgBox;
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setText(tr("次回以降は確認しない"));
        msgBox.setCheckBox(checkBox);
        msgBox.setText(tr("終了しますか？"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

        if (msgBox.exec() == QMessageBox::No) {
            event->ignore();
            return;
        }
        settings.setValue(IniKey_ConfirmExit, !checkBox->isChecked());
    }

    settings.setValue(IniKey_WindowGeometry, saveGeometry());
    settings.setValue(iniKey_WindowState, saveState());
    QMainWindow::closeEvent(event);
}
