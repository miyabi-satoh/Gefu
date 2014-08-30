#include "common.h"
#include "version.h"
#include "preferencedialog.h"
#include "folderview.h"
#include "searchbox.h"
#include "locationbox.h"
#include "copymoveworker.h"
#include "operationdialog.h"
#include "overwritedialog.h"
#include "deleteworker.h"
#include "renameworker.h"
#include "renamesingledialog.h"
#include "renamemultidialog.h"
#include "historydialog.h"
#include "sortdialog.h"
#include "simpletextview.h"
#include "simpleimageview.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bookmarkdialog.h"

#include <QCheckBox>
#include <QCloseEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QInputDialog>
#include <QProcess>
#include <QDesktopServices>
#include <QClipboard>
#include <QFileDialog>
#include <folderpanel.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_focusedView(NULL),
    m_overwriteDialog(NULL),
    m_viewMode(ModeBasic)
{
    qDebug() << ">>>>>>>>>> MainWindowの構築開始 <<<<<";
    ui->setupUi(this);

    m_overwriteDialog = new OverWriteDialog(this);

    initActionConnections();
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChange(QWidget*,QWidget*)));
    connect(ui->pane3->textView(), SIGNAL(viewFinished()), this, SLOT(viewFinish()));
    connect(ui->pane3->imageView(), SIGNAL(viewFinished()), this, SLOT(viewFinish()));
    connect(ui->pane1->textView(), SIGNAL(fileInfo(QString)), this, SLOT(showFileInfo(QString)));
    connect(ui->pane2->textView(), SIGNAL(fileInfo(QString)), this, SLOT(showFileInfo(QString)));
    connect(ui->pane3->textView(), SIGNAL(fileInfo(QString)), this, SLOT(showFileInfo(QString)));
    connect(ui->pane1->imageView(), SIGNAL(fileInfo(QString)), this, SLOT(showFileInfo(QString)));
    connect(ui->pane2->imageView(), SIGNAL(fileInfo(QString)), this, SLOT(showFileInfo(QString)));
    connect(ui->pane3->imageView(), SIGNAL(fileInfo(QString)), this, SLOT(showFileInfo(QString)));

    // ビューアは初期状態で非表示
    ui->pane3->setVisible(false);

    // ステータスバーにウィジェットを設定する
    QLabel *label = new QLabel("", this);
    label->setAlignment(Qt::AlignRight);
    label->setObjectName("Right");
    ui->statusBar->addPermanentWidget(label, 0);

    for (int i = 1; i <= 2; i++) {
        // pane1 or pane2
        AnyView *anyView = findChild<AnyView*>(QString("pane%1").arg(i));
        Q_CHECK_PTR(anyView);

        FolderPanel *fp = anyView->findChild<FolderPanel*>("folderPanel");
        Q_CHECK_PTR(fp);

        // FolderPanelとFolderViewの名前を変更する
        fp->setObjectName(QString("folderPanel%1").arg(i));
        fp->folderView()->setObjectName(QString("folderView%1").arg(i));

        fp->initialize(this, i == 1);
    }

    QSettings settings;
    // ブックマークメニューを初期化する
    initBookmark();

    // メニュー項目の状態を初期化する
    ui->view_Hidden->setChecked(settings.value(IniKey_ShowHidden).toBool());
    ui->view_System->setChecked(settings.value(IniKey_ShowSystem).toBool());
    ui->action_OpenEditor->setEnabled(!settings.value(IniKey_EditorPath).toString().isEmpty());
    ui->action_OpenTerminal->setEnabled(!settings.value(IniKey_TerminalPath).toString().isEmpty());

    // 追加のショートカットキーを設定する
    QList<QKeySequence> shortcuts;
    shortcuts = ui->action_Open->shortcuts();
    shortcuts.append(QKeySequence("M"));
    ui->action_Open->setShortcuts(shortcuts);

    shortcuts = ui->action_Exec->shortcuts();
    shortcuts.append(QKeySequence("Shift+M"));
    ui->action_Exec->setShortcuts(shortcuts);

    // MacだとShift+の形で認識されてしまうもの
    shortcuts = ui->view_FontSizeUp->shortcuts();
    shortcuts.append(QKeySequence("Shift++"));
    ui->view_FontSizeUp->setShortcuts(shortcuts);

    shortcuts = ui->view_Filter->shortcuts();
    shortcuts.append(QKeySequence("Shift+*"));
    ui->view_Filter->setShortcuts(shortcuts);

    shortcuts = ui->action_OpenTerminal->shortcuts();
    shortcuts.append(QKeySequence("Shift+>"));
    ui->action_OpenTerminal->setShortcuts(shortcuts);

    shortcuts = ui->help_About->shortcuts();
    shortcuts.append(QKeySequence("Shift+?"));
    ui->help_About->setShortcuts(shortcuts);

    // ウィンドウタイトルを設定する
    setWindowTitle(tr("げふぅ v%1").arg(VERSION_VALUE));

    // ウィンドウアイコンを設定する
#if defined(Q_OS_MAC)
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

    ui->pane2->changeView(AnyView::ViewFolder);
    ui->pane2->repaint();

    ui->pane1->changeView(AnyView::ViewFolder);
    ui->pane1->repaint();

    qDebug() << ">>>>>>>>>> MainWindowの構築終了 <<<<<";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::focusChange(QWidget *old, QWidget *now)
{
    qDebug() << ">>>>> フォーカス変更";
    if (old) qDebug() << "old is" << old->objectName();
    if (now) qDebug() << "now is" << now->objectName();

    // 検索ボックスがフォーカスを失ったら、非表示にする
    if (old == ui->pane1->folderPanel()->searchBox() ||
        old == ui->pane2->folderPanel()->searchBox())
    {
        ui->action_Search->setChecked(false);
    }

    // フォルダビューにフォーカスが移ったら、ステータスバーの表示を更新する
    if (now == ui->pane1->folderPanel()->folderView() ||
        now == ui->pane2->folderPanel()->folderView())
    {
        FolderView *view = static_cast<FolderView*>(now);
        ui->statusBar->showMessage(view->currentItem().absoluteFilePath());
    }

    if (now) {
        updateActions();
    }
}

void MainWindow::executeCommand()
{
    qDebug() << "MainWindow::executeCommand";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QFileInfoList list = view->selectedItems();
    QString command = QString::null;
    foreach (const QFileInfo &info, list) {
#ifdef Q_OS_MAC
        QString path = info.absoluteFilePath();
#else
        QString path = info.fileName();
#endif
        if (info.isExecutable()) {
            command = QQ(path) + " " + command;
        }
        else {
            command += " " + QQ(path);
        }
    }

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setWindowTitle(tr("コマンドを実行"));
    dlg.setLabelText(tr("コマンド："));
    dlg.setTextValue(command);
    dlg.resize(width() * 0.8, dlg.height());

    int ret = dlg.exec();
    command = dlg.textValue();
    if (ret == QDialog::Accepted && !command.isEmpty()) {
        startProcess(command, view->dir(), tr("コマンドの実行に失敗しました。"));
    }
}

void MainWindow::historyBack()
{
    qDebug() << "MainWindow::historyBack();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    if (view->historyBack()) {
        updateActions();
    }
}

void MainWindow::historyForward()
{
    qDebug() << "MainWindow::historyForward();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    if (view->historyForward()) {
        updateActions();
    }
}

void MainWindow::markAll()
{
    qDebug() << "MainWindow::markAll();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->setCheckStateAll(true);
}

void MainWindow::markAllFiles()
{
    qDebug() << "MainWindow::markAllFiles();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->setCheckStateAllFiles();
}

void MainWindow::markAllOff()
{
    qDebug() << "MainWindow::markAllOff();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->setCheckStateAll(false);
}

void MainWindow::markInvert()
{
    qDebug() << "MainWindow::markInvert();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->invertCheckState();
}

void MainWindow::markToggle()
{
    qDebug() << "MainWindow::markToggle();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->toggleCheckState(view->currentIndex());
}

void MainWindow::moveItems()
{
    qDebug() << "MainWindow::moveItems";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QFileInfoList list = view->selectedItems();
    if (list.isEmpty()) {
        return;
    }

    QSettings settings;
    if (settings.value(IniKey_ConfirmMove).toBool()) {
        if (QMessageBox::question(this, tr("確認"), tr("移動を実行しますか？"))
            != QMessageBox::Yes)
        {
            return;
        }
    }

    // 上書き確認ダイアログを初期化する
    m_overwriteDialog->reset();

    // ワーカースレッドを作成する
    FolderView *other = otherSideFolderView(view);
    CopyMoveWorker *worker = new CopyMoveWorker();
    connect(worker, SIGNAL(askOverWrite(QString*,QString*,QString,QString)),
            this, SLOT(askOverWrite(QString*,QString*,QString,QString)));
    worker->setCopyList(&list);
    worker->setTargetDir(other->dir());
    worker->setMoveMode(true);

    // 進捗ダイアログを表示して、処理を開始する
    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("移動"));
    opDlg.setWorker(worker);
    opDlg.setAutoClose(settings.value(IniKey_AutoCloseMove).toBool());

    int row = view->currentIndex().row();

    opDlg.exec();

    if (row >= view->model()->rowCount()) {
        row = view->model()->rowCount() - 1;
    }
    //view->selectRow(row);
    view->setCurrentIndex(view->model()->index(row, 1));

    settings.setValue(IniKey_AutoCloseMove, opDlg.autoClose());
}

void MainWindow::leftKeyPress()
{
    qDebug() << "MainWindow::leftKeyPress();";

    if (ui->pane3->textView()->hasFocus()) {
        QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_PageUp, Qt::NoModifier);
        QApplication::sendEvent(ui->pane3->textView(), &event);
    }
    else if (ui->pane1->folderPanel()->folderView()->hasFocus()) {
        setPathToParent();
    }
    else if (ui->pane2->folderPanel()->folderView()->hasFocus()) {
        ui->pane1->folderPanel()->folderView()->setFocus();
    }
}

void MainWindow::rightKeyPress()
{
    qDebug() << "MainWindow::rightKeyPress();";

    if (ui->pane3->textView()->hasFocus()) {
        QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_PageDown, Qt::NoModifier);
        QApplication::sendEvent(ui->pane3->textView(), &event);
    }
    else if (ui->pane1->folderPanel()->folderView()->hasFocus()) {
        ui->pane2->folderPanel()->folderView()->setFocus();
    }
    else if (ui->pane2->folderPanel()->folderView()->hasFocus()) {
        setPathToParent();
    }
}

void MainWindow::showFileInfo(const QString &str)
{
    qDebug() << "MainWindow::showFileInfo();" << str;
    QLabel *label = ui->statusBar->findChild<QLabel*>("Right");
    Q_CHECK_PTR(label);

    label->setText(str);
}

void MainWindow::chooseFolder()
{
    qDebug() << "MainWindow::chooseFolder();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QString path = QFileDialog::getExistingDirectory(
                this, tr("フォルダを選択"), view->dir());
    if (!path.isEmpty()) {
        view->setPath(path, true);
        updateActions();
    }
}

void MainWindow::copyFilenameToClipboard()
{
    qDebug() << "MainWindow::copyFilenameToClipboard();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(view->currentItem().fileName());
}

void MainWindow::copyFullpathTpClipboard()
{
    qDebug() << "MainWindow::copyFullpathTpClipboard();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(view->currentItem().absoluteFilePath());
}

void MainWindow::askOverWrite(QString *copyMethod,
                              QString *alias,
                              const QString &srcPath,
                              const QString &tgtPath)
{
    qDebug() << "MainWindow::askOverWrite();";

    CopyMoveWorker *worker = static_cast<CopyMoveWorker*>(sender());
    if (!m_overwriteDialog->isKeepSetting() ||
        m_overwriteDialog->copyMethod() == "rbRename")
    {
        m_overwriteDialog->setFileInfo(srcPath, tgtPath);
        if (m_overwriteDialog->exec() == QDialog::Rejected) {
            worker->requestStop();
        }
    }
    *copyMethod = m_overwriteDialog->copyMethod();
    *alias = m_overwriteDialog->alias();

    worker->endAsking();
}

void MainWindow::currentChange(const QFileInfo &info)
{
    qDebug() << ">>>>> カーソル変更 <<<<<";

    FolderView *view = static_cast<FolderView*>(sender());
    Q_CHECK_PTR(view);

    if (!view->hasFocus()) {
        return;
    }

    // ステータスバーにカーソルのフルパスを表示する
    ui->statusBar->showMessage(info.absoluteFilePath());

    // ハーフモードで変更された場合
    if (m_viewMode & ModeHalfView) {
        AnyView *otherSide = static_cast<AnyView*>(
                    otherSideFolderView(view)->parent()->parent());
        Q_CHECK_PTR(otherSide);

        if (!otherSide->setViewItem(view->currentItem())) {
            showFileInfo("");
        }
    }

    updateActions();
}

void MainWindow::dropAccept(const QFileInfoList &list)
{
    qDebug() << "MainWindow::dropAccept();";

    FolderView *view = static_cast<FolderView*>(sender());
    Q_CHECK_PTR(view);

    copyItems(list, view->dir());
}

void MainWindow::copyItems(const QFileInfoList &list, const QString &tgtDir)
{
    qDebug() << "MainWindow::copyItems();";

    QSettings settings;
    if (settings.value(IniKey_ConfirmCopy).toBool()) {
        if (QMessageBox::question(this, tr("確認"), tr("コピーを実行しますか？"))
            != QMessageBox::Yes)
        {
            return;
        }
    }

    // 上書き確認ダイアログを初期化する
    m_overwriteDialog->reset();

    // ワーカースレッドを作成する
    CopyMoveWorker *worker = new CopyMoveWorker();
    connect(worker, SIGNAL(askOverWrite(QString*,QString*,QString,QString)),
            this, SLOT(askOverWrite(QString*,QString*,QString,QString)));
    worker->setCopyList(&list);
    worker->setTargetDir(tgtDir);
    worker->setMoveMode(false);

    // 進捗ダイアログを表示して、処理を開始する
    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("コピー"));
    opDlg.setWorker(worker);
    opDlg.setAutoClose(settings.value(IniKey_AutoCloseCopy).toBool());

    opDlg.exec();

    settings.setValue(IniKey_AutoCloseCopy, opDlg.autoClose());
}

void MainWindow::copyItems()
{
    qDebug() << "MainWindow::copyItems";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QFileInfoList list = view->selectedItems();
    if (list.isEmpty()) {
        return;
    }

    QString tgtPath = otherSideFolderView(view)->dir();
    copyItems(list, tgtPath);
}

void MainWindow::createFile()
{
    qDebug() << "MainWindow::createFile";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    bool bOk;
    QString name = QInputDialog::getText(
                this, tr("ファイルを作成"), tr("ファイル名："),
                QLineEdit::Normal, "", &bOk);
    if (!bOk || name.isEmpty()) {
        return;
    }

    QDir dir(view->dir());
    QFile file(dir.absoluteFilePath(name));
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("ファイルの作成に失敗しました。"));
    }
    else {
        file.close();

        QSettings settings;
        if (settings.value(IniKey_OpenAfterCreateFile).toBool()) {
            openEditor(dir.absoluteFilePath(name));
        }

        view->refresh();
        view->searchItem(name);
    }
}

void MainWindow::createFolder()
{
    qDebug() << "MainWindow::createFolder";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    bool bOk;
    QString name = QInputDialog::getText(
                this, tr("フォルダを作成"), tr("フォルダ名："),
                QLineEdit::Normal, "", &bOk);
    if (!bOk || name.isEmpty()) {
        return;
    }

    QDir dir(view->dir());
    if (!dir.mkpath(name)) {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("フォルダの作成に失敗しました。"));
    }
    else {
        QSettings settings;
        if (settings.value(IniKey_MoveAfterCreateFolder).toBool()) {
            view->setPath(dir.absoluteFilePath(name), true);
        }

        view->refresh();
        view->searchItem(name);
    }
}

void MainWindow::deleteItems()
{
    qDebug() << "MainWindow::deleteItems";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QFileInfoList list = view->selectedItems();
    if (list.isEmpty()) {
        return;
    }

    QSettings settings;
    if (settings.value(IniKey_ConfirmDelete).toBool()) {
        QString msg;
        if (list.size() == 1) {
            msg = list[0].fileName();
        }
        else {
            msg = tr("%1個のアイテム").arg(list.size());
        }
        int ret = QMessageBox::question(
                    this, tr("確認"),
                    msg + tr("を削除しますか？"));
        if (ret != QMessageBox::Yes) {
            return;
        }
    }

    DeleteWorker *worker = new DeleteWorker();
    worker->setDeleteList(&list);

    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("削除"));
    opDlg.setWorker(worker);
    opDlg.setAutoClose(settings.value(IniKey_AutoCloseDelete).toBool());

    int row = view->currentIndex().row();

    opDlg.exec();
    view->refresh();

    if (row >= view->model()->rowCount()) {
        row = view->model()->rowCount() - 1;
    }
//    view->selectRow(row);
    view->setCurrentIndex(view->model()->index(row, 1));

    settings.setValue(IniKey_AutoCloseDelete, opDlg.autoClose());
}

void MainWindow::open(const QModelIndex &index)
{
    Q_UNUSED(index);
    qDebug() << "MainWindow::open";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QFileInfo info = view->currentItem();
    if (info.isDir()) {
        view->setPath(info.absoluteFilePath(), true);
        updateActions();
        return;
    }

    setUpdatesEnabled(false);

    m_focusedView = view;
    if (ui->pane3->setViewItem(info)) {
        ui->pane3->setVisible(true);
        ui->splitter->setVisible(false);

        setViewMode(ModeFullView);
    }
    else {
        ui->pane3->setVisible(false);
    }

    setUpdatesEnabled(true);
}

void MainWindow::openEditor(const QString &path)
{
    qDebug() << "MainWindow::openEditor";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QSettings settings;
    QString exe = settings.value(IniKey_EditorPath).toString();
    if (exe.isEmpty()) {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("外部エディタのパスが未定義です。"));
        return;
    }

    QFileInfoList list;
    if (path.isEmpty()) {
        list = view->selectedItems();
    }
    else {
        list << path;
    }

    foreach (const QFileInfo &info, list) {
        QString opt = settings.value(IniKey_EditorOption).toString();
        replaceVars(opt, info);

#ifdef Q_OS_MAC
        QString command = "open -a " + exe + " " + opt;
#else
        QString command = QQ(exe) + " " + opt;
#endif
        if (!startProcess(command, info.absolutePath(), tr("外部エディタの起動に失敗しました。"))) {
            break;
        }
    }
}

void MainWindow::openTerminal()
{
    qDebug() << "MainWindow::openTerminal";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QSettings settings;
    QString exe = settings.value(IniKey_TerminalPath).toString();
    if (exe.isEmpty()) {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("ターミナルのパスが未定義です。"));
        return;
    }

    foreach (const QFileInfo &info, view->selectedItems()) {
        QString opt = settings.value(IniKey_TerminalOption).toString();
        replaceVars(opt, info);

#ifdef Q_OS_MAC
        QString command = "open -a " + QQ(exe) + " --args " + opt;
#else
        QString command = QQ(exe) + " " + opt;
#endif
        if (!startProcess(command, info.absolutePath(), tr("ターミナルの起動に失敗しました。"))) {
            break;
        }
        Sleep(1000);
    }
}

void MainWindow::openBookmark()
{
    qDebug() << "MainWindow::openBookmark();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QAction *action = qobject_cast<QAction*>(sender());
    Q_CHECK_PTR(action);

    QSettings settings;
    int i = action->data().toInt();
    view->setPath(settings.value(IniKey_BookmarkEntryPath(i)).toString(), true);
}

void MainWindow::refresh()
{
    qDebug() << "MainWindow::refresh();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->refresh();
}

void MainWindow::renameItems()
{
    qDebug() << "MainWindow::renameItems";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QFileInfoList list = view->selectedItems();
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
    dlg->setWorkingDirectory(view->dir());
    dlg->setNames(list);
    int dlgResult = dlg->exec();
    if (dlgResult != QDialog::Accepted || dlg->renameMap().isEmpty()) {
        return;
    }

    QSettings settings;
    if (settings.value(IniKey_ConfirmRename).toBool()) {
        int ret = QMessageBox::question(this, tr("確認"),
                                        tr("名前の変更を実行しますか？"));
        if (ret != QMessageBox::Yes) {
            return;
        }
    }

    RenameWorker *worker = new RenameWorker();
    worker->setRenameMap(&dlg->renameMap());

    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("名前を変更"));
    opDlg.setWorker(worker);
    opDlg.setAutoClose(settings.value(IniKey_AutoCloseRename).toBool());

    opDlg.exec();
    view->refresh();

    QFileInfo info(dlg->renameMap().first());
    view->searchItem(info.fileName());

    settings.setValue(IniKey_AutoCloseRename, opDlg.autoClose());
}

void MainWindow::shellExecute()
{
    qDebug() << "MainWindow::shellExecute";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    foreach (const QFileInfo &info, view->selectedItems()) {
        QString path = QDir::toNativeSeparators(info.absoluteFilePath());
        QDesktopServices::openUrl(QUrl("file:///" + path));
    }
}

void MainWindow::showBookmarkDialog()
{
    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    BookmarkDialog dlg(this);
    dlg.setEditMode(false);
    if (dlg.exec() == QDialog::Accepted) {
        int n = dlg.selectedIndex();

        QSettings settings;
        view->setPath(settings.value(IniKey_BookmarkEntryPath(n)).toString(), true);
    }
}

void MainWindow::editBookmark()
{
    qDebug() << "MainWindow::showBookmarkDialog();";

    BookmarkDialog dlg(this);
    dlg.setEditMode(true);

    if (dlg.exec() == QDialog::Accepted) {
        initBookmark();
    }
}

void MainWindow::showFilterDialog()
{
    qDebug() << "MainWindow::showFilterDialog();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QString filters = view->nameFilters().join(" ");

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setWindowTitle(tr("フィルタを設定"));
    dlg.setLabelText(tr("フィルタ："));
    dlg.setTextValue(filters);
    dlg.resize(width() * 0.8, dlg.height());

    if (dlg.exec() == QDialog::Accepted) {
        static_cast<FolderPanel*>(view->parent())->setNameFilters(dlg.textValue());
        int row = view->currentIndex().row();

        view->refresh();

        if (row >= view->model()->rowCount()) {
            row = view->model()->rowCount() - 1;
        }
//        view->selectRow(row);
        view->setCurrentIndex(view->model()->index(row, 1));

    }
}

void MainWindow::showHistoryDialog()
{
    qDebug() << "MainWindow::showHistoryDialog();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    FolderView *vOther = otherSideFolderView(view);

    HistoryDialog dlg(this);
    if (view->side() == "Left") {
        dlg.setDefaultLeft(true);
        dlg.setHistory(view->history(), vOther->history());
    }
    else {
        dlg.setDefaultLeft(false);
        dlg.setHistory(vOther->history(), view->history());
    }

    if (dlg.exec() == QDialog::Accepted) {
        if (view->side() == dlg.selectedSide()) {
            view->setHistoryIndexAt(dlg.selectedIndex());
        }
        else {
            view->setPath(vOther->history()->at(dlg.selectedIndex()), true);
        }
    }
}

void MainWindow::showSortDialog()
{
    qDebug() << "MainWindow::showSortDialog();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    SortDialog dlg(this);
    dlg.setRightOrLeft(view->side());

    if (dlg.exec() == QDialog::Accepted) {
        view->setSorting();
        view->refresh();
    }
}

void MainWindow::swapView()
{
    qDebug() << "MainWindow::swapView();";

    QString dir1 = ui->pane1->folderPanel()->folderView()->dir();
    QString dir2 = ui->pane2->folderPanel()->folderView()->dir();

    ui->pane1->folderPanel()->folderView()->setPath(dir2, true);
    ui->pane2->folderPanel()->folderView()->setPath(dir1, true);

    updateActions();
}

void MainWindow::switchHalfMode(bool checked)
{
    qDebug() << "MainWindow::switchHalfMode();" << checked;

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    AnyView *pane = static_cast<AnyView*>(
                otherSideFolderView(view)->parent()->parent());
    Q_CHECK_PTR(pane);

    if (checked) {
        // ハーフモードへ移行する
        setViewMode(ModeHalfView);
        pane->setViewItem(view->currentItem());
    }
    else {
        // ハーフモードを解除する
        setViewMode(ModeBasic);
        pane->changeView(AnyView::ViewFolder);
    }
    updateActions();
}

//void MainWindow::searchItem(const QString &text)
//{
//    qDebug() << "MainWindow::searchItem" << text;

//    SearchBox *box = qobject_cast<SearchBox*>(qApp->focusWidget());
//    Q_CHECK_PTR(box);

//    if (text.right(1) == "/") {
//        // '/'が入力されたら、検索終了
//        box->setText(text.left(text.length() - 1));
//        ui->action_Search->setChecked(false);
//    }
//    else {
//        FolderPanel *fp = qobject_cast<FolderPanel*>(box->parent());
//        Q_CHECK_PTR(fp);

//        fp->folderView()->searchItem(box->text());
//    }
//}

void MainWindow::searchNext()
{
    qDebug() << "MainWindow::searchNext";

    SearchBox *box = qobject_cast<SearchBox*>(qApp->focusWidget());
    Q_CHECK_PTR(box);

    FolderPanel *fp = qobject_cast<FolderPanel*>(box->parent());
    Q_CHECK_PTR(fp);

    fp->folderView()->searchNext(box->text());
}

void MainWindow::searchPrev()
{
    qDebug() << "MainWindow::searchPrev";

    SearchBox *box = qobject_cast<SearchBox*>(qApp->focusWidget());
    Q_CHECK_PTR(box);

    FolderPanel *fp = qobject_cast<FolderPanel*>(box->parent());
    Q_CHECK_PTR(fp);

    fp->folderView()->searchPrev(box->text());
}

void MainWindow::setCursorToBegin()
{
    qDebug() << "MainWindow::setCursorToBegin();";

    QKeyEvent event1 = QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier);
    QApplication::sendEvent(QApplication::focusWidget(), &event1);

    QKeyEvent event2 = QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::ControlModifier);
    QApplication::sendEvent(QApplication::focusWidget(), &event2);
}

void MainWindow::cursorDown()
{
    qDebug() << "MainWindow::cursorDown();";

    QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    QApplication::sendEvent(QApplication::focusWidget(), &event);
}

void MainWindow::cursorUp()
{
    qDebug() << "MainWindow::cursorUp();";

    QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    QApplication::sendEvent(QApplication::focusWidget(), &event);
}

void MainWindow::setCursorToEnd()
{
    qDebug() << "MainWindow::setCursorToEnd();";

    QKeyEvent event1 = QKeyEvent(QEvent::KeyPress, Qt::Key_End, Qt::NoModifier);
    QApplication::sendEvent(QApplication::focusWidget(), &event1);

    QKeyEvent event2 = QKeyEvent(QEvent::KeyPress, Qt::Key_End, Qt::ControlModifier);
    QApplication::sendEvent(QApplication::focusWidget(), &event2);
}

void MainWindow::setCursorToBeginOther()
{
    qDebug() << "MainWindow::setCursorToBeginOther();";

    QKeyEvent event1 = QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::NoModifier);
    sendEventOther(&event1);

    QKeyEvent event2 = QKeyEvent(QEvent::KeyPress, Qt::Key_Home, Qt::ControlModifier);
    sendEventOther(&event2);
}

void MainWindow::cursorDownOther()
{
    qDebug() << "MainWindow::cursorDownOther();";

    QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    sendEventOther(&event);
}

void MainWindow::cursorUpOther()
{
    qDebug() << "MainWindow::cursorUpOther();";

    QKeyEvent event = QKeyEvent(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    sendEventOther(&event);
}

void MainWindow::setCursorToEndOther()
{
    qDebug() << "MainWindow::setCursorToEndOther();";

    QKeyEvent event1 = QKeyEvent(QEvent::KeyPress, Qt::Key_End, Qt::NoModifier);
    sendEventOther(&event1);

    QKeyEvent event2 = QKeyEvent(QEvent::KeyPress, Qt::Key_End, Qt::ControlModifier);
    sendEventOther(&event2);
}

void MainWindow::setFontSizeDown()
{
    qDebug() << "MainWindow::setFontSizeDown();";

    changeFontSize(-1);
}

void MainWindow::setFontSizeUp()
{
    qDebug() << "MainWindow::setFontSizeUp();";

    changeFontSize(1);
}

void MainWindow::changeFontSize(int diff)
{
    qDebug() << "MainWindow::changeFontSize(;";

    QSettings settings;
    QFont font;

    // フォルダビューのフォントサイズ変更
    if (ui->pane1->folderPanel()->folderView()->hasFocus() ||
        ui->pane2->folderPanel()->folderView()->hasFocus())
    {
        font = settings.value(IniKey_ViewFont).value<QFont>();
        font.setPointSize(font.pointSize() + diff);
        settings.setValue(IniKey_ViewFont, font);

        ui->pane1->folderPanel()->folderView()->updateAppearance();
        ui->pane2->folderPanel()->folderView()->updateAppearance();
    }

    // テキストビューのフォントサイズ変更
    if (ui->pane1->textView()->hasFocus() ||
        ui->pane2->textView()->hasFocus() ||
        ui->pane3->textView()->hasFocus())
    {
        font = settings.value(IniKey_ViewerFont).value<QFont>();
        font.setPointSize(font.pointSize() + diff);
        settings.setValue(IniKey_ViewerFont, font);

        ui->pane1->textView()->updateAppearance();
        ui->pane2->textView()->updateAppearance();
        ui->pane3->textView()->updateAppearance();
    }

    if (ui->pane1->imageView()->hasFocus() ||
        ui->pane2->imageView()->hasFocus() ||
        ui->pane3->imageView()->hasFocus())
    {
        ui->pane1->imageView()->changeScale(diff > 0);
        ui->pane2->imageView()->changeScale(diff > 0);
        ui->pane3->imageView()->changeScale(diff > 0);
    }
}

void MainWindow::initBookmark()
{
    QSettings settings;
    QFileIconProvider iconProvider;

    ui->menu_Bookmark->clear();
    ui->menu_Bookmark->addAction(ui->bookmark_Edit);
    ui->menu_Bookmark->addSeparator();
    int i = 0;
    while (!settings.value(IniKey_BookmarkEntryName(i), "").toString().isEmpty()) {
        QString path = settings.value(IniKey_BookmarkEntryPath(i)).toString();
        QAction *action = new QAction(this);
        action->setText(settings.value(IniKey_BookmarkEntryName(i)).toString());
        action->setData(i);
        action->setIcon(iconProvider.icon(QFileInfo(path)));
        ui->menu_Bookmark->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(openBookmark()));
        i++;
    }
}

void MainWindow::sendEventOther(QEvent *event)
{
    qDebug() << "MainWindow::sendEventOther();";

    QWidget *widget = NULL;

    if (m_viewMode & ModeBasic) {
        // 検索ボックスにフォーカスがある場合も考慮して、
        // FolderPanalを介してViewを取得する
        FolderPanel *fp = qobject_cast<FolderPanel*>(qApp->focusWidget()->parentWidget());
        Q_CHECK_PTR(fp);

        widget = otherSideFolderView(fp->folderView());
    }
    else if (m_viewMode & ModeHalfView) {
        AnyView *focusedView = qobject_cast<AnyView*>(qApp->focusWidget()->parentWidget());
        if (!focusedView) {
            focusedView = qobject_cast<AnyView*>(qApp->focusWidget()->parentWidget()->parentWidget());
        }
        Q_CHECK_PTR(focusedView);

        if (focusedView == ui->pane1) {
            widget = ui->pane2->visibleView();
        }
        else {
            Q_ASSERT(focusedView == ui->pane2);
            widget = ui->pane1->visibleView();
        }
    }
    else {
        return;
    }

    Q_CHECK_PTR(widget);
    QApplication::sendEvent(widget, event);
}

void MainWindow::setPathFromOther()
{
    qDebug() << "MainWindow::setPathFromOther();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    FolderView *other = otherSideFolderView(view);

    view->setPath(other->dir(), true);
}

void MainWindow::setPathToHome()
{
    qDebug() << "MainWindow::setPathToHome();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->setPath(QDir::homePath(), true);
}

void MainWindow::setPathToOther()
{
    qDebug() << "MainWindow::setPathToOther();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    FolderView *other = otherSideFolderView(view);

    other->setPath(view->dir(), true);
}

void MainWindow::setPathToParent()
{
    qDebug() << "MainWindow::setPathToParent();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    QDir dir(view->dir());
    if (!dir.isRoot()) {
        dir.cdUp();
        view->setPath(dir.absolutePath(), true);
    }
}

void MainWindow::setPathToRoot()
{
    qDebug() << "MainWindow::setPathToRoot();";

    FolderView *view = static_cast<FolderView*>(qApp->focusWidget());
    Q_CHECK_PTR(view);

    view->setPath(QDir::rootPath(), true);
}

void MainWindow::toggleSearchBox(bool checked)
{
    qDebug() << "MainWindow::toggleSearchBox" << checked;

    FolderView *view;
    SearchBox *box;

    if (checked) {
        setViewMode(m_viewMode | ModeSearch);
        view = static_cast<FolderView*>(qApp->focusWidget());
        Q_CHECK_PTR(view);

        box = view->parent()->findChild<SearchBox*>("searchBox");
        Q_CHECK_PTR(box);

        box->setVisible(true);
        box->setFocus();
        box->selectAll();
    }
    else {
        setViewMode(m_viewMode ^ ModeSearch);
        if (ui->pane1->folderPanel()->searchBox()->isVisible()) {
            box = ui->pane1->folderPanel()->searchBox();
            view = ui->pane1->folderPanel()->folderView();
        }
        else {
            Q_ASSERT(ui->pane2->folderPanel()->searchBox()->isVisible());
            box = ui->pane2->folderPanel()->searchBox();
            view = ui->pane2->folderPanel()->folderView();
        }

        if (box->hasFocus()) {
            view->setFocus();
        }
        box->setVisible(false);
    }
}

void MainWindow::showPreferenceDialog()
{
    qDebug() << "MainWindow::showPreferenceDialog";

    PreferenceDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        bool dark1 = !ui->pane1->folderPanel()->folderView()->hasFocus();
        bool dark2 = !ui->pane2->folderPanel()->folderView()->hasFocus();

        ui->pane1->folderPanel()->locationBox()->updateAppearance(dark1);
        ui->pane2->folderPanel()->locationBox()->updateAppearance(dark2);
        ui->pane1->folderPanel()->folderView()->updateAppearance(dark1);
        ui->pane2->folderPanel()->folderView()->updateAppearance(dark2);
        ui->pane1->textView()->updateAppearance();
        ui->pane2->textView()->updateAppearance();
        ui->pane3->textView()->updateAppearance();

        updateActions();
    }
}

void MainWindow::toggleShowHiddenFiles(bool checked)
{
    qDebug() << "MainWindow::toggleShowHiddenFiles" << checked;

    ui->pane1->folderPanel()->folderView()->setFilter(QDir::Hidden, checked);
    ui->pane1->folderPanel()->folderView()->refresh();

    ui->pane2->folderPanel()->folderView()->setFilter(QDir::Hidden, checked);
    ui->pane2->folderPanel()->folderView()->refresh();
}

void MainWindow::toggleShowSystemFiles(bool checked)
{
    qDebug() << "MainWindow::toggleShowSystemFiles" << checked;

    ui->pane1->folderPanel()->folderView()->setFilter(QDir::System, checked);
    ui->pane1->folderPanel()->folderView()->refresh();

    ui->pane2->folderPanel()->folderView()->setFilter(QDir::System, checked);
    ui->pane2->folderPanel()->folderView()->refresh();
}

void MainWindow::showContextMenu(QContextMenuEvent *event)
{
    qDebug() << "MainWindow::showContextMenu();";

    FolderView *view = static_cast<FolderView*>(sender());
    Q_CHECK_PTR(view);

    QModelIndex index = view->indexAt(event->pos());

    QMenu menu(this);
    if (index.isValid()) {
        menu.addAction(ui->action_Open);
        menu.addAction(ui->action_Exec);
        menu.addAction(ui->action_OpenEditor);
        menu.addAction(ui->action_OpenTerminal);
        menu.addSeparator();
        menu.addAction(ui->cmd_Copy);
        menu.addAction(ui->cmd_Move);
        menu.addSeparator();
        menu.addAction(ui->cmd_Delete);
        menu.addSeparator();
        menu.addAction(ui->copy_Filename);
        menu.addAction(ui->copy_Fullpath);
        menu.addSeparator();
        menu.addAction(ui->cmd_Rename);
    }
    else {
        menu.addAction(ui->move_Back);
        menu.addAction(ui->move_Forward);
        menu.addSeparator();
        menu.addAction(ui->move_Parent);
        menu.addAction(ui->move_Home);
        menu.addAction(ui->move_Root);
        menu.addAction(ui->move_Jump);
        menu.addSeparator();
        menu.addAction(ui->cmd_NewFile);
        menu.addAction(ui->cmd_NewFolder);
    }

    menu.exec(event->globalPos());
}

void MainWindow::checkUpdate(bool silent)
{
    qDebug() << "MainWindow::checkUpdate()" << silent;

    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    if (silent) {
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkUpdateFinishedSilent(QNetworkReply*)));
    }
    else {
        connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(checkUpdateFinished(QNetworkReply*)));
    }
#ifdef Q_OS_WIN
    manager->get(QNetworkRequest(QUrl("http://gefu.sourceforge.jp/gefu_latest_win.html")));
#elif defined(Q_OS_MAC)
    manager->get(QNetworkRequest(QUrl("http://gefu.sourceforge.jp/gefu_latest_mac.html")));
#endif
}

void MainWindow::checkUpdateFinishedSilent(QNetworkReply *reply)
{
    qDebug() << "MainWindow::checkUpdateFinishedSilent";

    checkUpdateFinished(reply, true);
}

void MainWindow::checkUpdateFinished(QNetworkReply *reply, bool silent)
{
    qDebug() << "MainWindow::checkUpdateFinished" << silent;

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

void MainWindow::viewFinish()
{
    qDebug() << "MainWindow::viewFinish();" << sender()->objectName();

    ui->statusBar->findChild<QLabel*>("Right")->setText("");
    ui->pane3->setVisible(false);
    ui->splitter->setVisible(true);
    m_focusedView->setFocus();
}

void MainWindow::initActionConnections()
{
    qDebug() << "MainWindow::initActionConnections";

    connect(ui->key_Left, SIGNAL(triggered()), this, SLOT(leftKeyPress()));
    connect(ui->key_Right, SIGNAL(triggered()), this, SLOT(rightKeyPress()));

    connect(ui->action_Command, SIGNAL(triggered()), this, SLOT(executeCommand()));
    connect(ui->action_Exec, SIGNAL(triggered()), this, SLOT(shellExecute()));
    connect(ui->action_Open, SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->action_OpenEditor, SIGNAL(triggered()), this, SLOT(openEditor()));
    connect(ui->action_OpenTerminal, SIGNAL(triggered()), this, SLOT(openTerminal()));
    connect(ui->action_Quit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->action_Search, SIGNAL(toggled(bool)), this, SLOT(toggleSearchBox(bool)));
    connect(ui->action_SearchNext, SIGNAL(triggered()), this, SLOT(searchNext()));
    connect(ui->action_SearchPrev, SIGNAL(triggered()), this, SLOT(searchPrev()));
    connect(ui->action_Setting, SIGNAL(triggered()), this, SLOT(showPreferenceDialog()));
    connect(ui->check_Update, SIGNAL(triggered()), this, SLOT(checkUpdate()));
    connect(ui->cmd_Copy, SIGNAL(triggered()), this, SLOT(copyItems()));
    connect(ui->cmd_Delete, SIGNAL(triggered()), this, SLOT(deleteItems()));
    connect(ui->cmd_Move, SIGNAL(triggered()), this, SLOT(moveItems()));
    connect(ui->cmd_NewFile, SIGNAL(triggered()), this, SLOT(createFile()));
    connect(ui->cmd_NewFolder, SIGNAL(triggered()), this, SLOT(createFolder()));
    connect(ui->cmd_Rename, SIGNAL(triggered()), this, SLOT(renameItems()));
    connect(ui->copy_Filename, SIGNAL(triggered()), this, SLOT(copyFilenameToClipboard()));
    connect(ui->copy_Fullpath, SIGNAL(triggered()), this, SLOT(copyFullpathTpClipboard()));
    connect(ui->help_About, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->mark_All, SIGNAL(triggered()), this, SLOT(markAll()));
    connect(ui->mark_AllFiles, SIGNAL(triggered()), this, SLOT(markAllFiles()));
    connect(ui->mark_AllOff, SIGNAL(triggered()), this, SLOT(markAllOff()));
    connect(ui->mark_Invert, SIGNAL(triggered()), this, SLOT(markInvert()));
    connect(ui->mark_Toggle, SIGNAL(triggered()), this, SLOT(markToggle()));
    connect(ui->move_Back, SIGNAL(triggered()), this, SLOT(historyBack()));
    connect(ui->move_Begin, SIGNAL(triggered()), this, SLOT(setCursorToBegin()));
    connect(ui->move_Down, SIGNAL(triggered()), this, SLOT(cursorDown()));
    connect(ui->move_End, SIGNAL(triggered()), this, SLOT(setCursorToEnd()));
    connect(ui->move_Up, SIGNAL(triggered()), this, SLOT(cursorUp()));
    connect(ui->move_BeginOther, SIGNAL(triggered()), this, SLOT(setCursorToBeginOther()));
    connect(ui->move_DownOther, SIGNAL(triggered()), this, SLOT(cursorDownOther()));
    connect(ui->move_EndOther, SIGNAL(triggered()), this, SLOT(setCursorToEndOther()));
    connect(ui->move_UpOther, SIGNAL(triggered()), this, SLOT(cursorUpOther()));
    connect(ui->move_Forward, SIGNAL(triggered()), this, SLOT(historyForward()));
    connect(ui->move_History, SIGNAL(triggered()), this, SLOT(showHistoryDialog()));
    connect(ui->move_Home, SIGNAL(triggered()), this, SLOT(setPathToHome()));
    connect(ui->move_Jump, SIGNAL(triggered()), this, SLOT(chooseFolder()));
    connect(ui->move_Parent, SIGNAL(triggered()), this, SLOT(setPathToParent()));
    connect(ui->move_Root, SIGNAL(triggered()), this, SLOT(setPathToRoot()));
    connect(ui->view_Filter, SIGNAL(triggered()), this, SLOT(showFilterDialog()));
    connect(ui->view_FontSizeDown, SIGNAL(triggered()), this, SLOT(setFontSizeDown()));
    connect(ui->view_FontSizeUp, SIGNAL(triggered()), this, SLOT(setFontSizeUp()));
    connect(ui->view_FromOther, SIGNAL(triggered()), this, SLOT(setPathFromOther()));
    connect(ui->view_HalfMode, SIGNAL(toggled(bool)), this, SLOT(switchHalfMode(bool)));
    connect(ui->view_Hidden, SIGNAL(toggled(bool)), this, SLOT(toggleShowHiddenFiles(bool)));
    connect(ui->view_Refresh, SIGNAL(triggered()), this, SLOT(refresh()));
    connect(ui->view_Sort, SIGNAL(triggered()), this, SLOT(showSortDialog()));
    connect(ui->view_Swap, SIGNAL(triggered()), this, SLOT(swapView()));
    connect(ui->view_System, SIGNAL(toggled(bool)), this, SLOT(toggleShowSystemFiles(bool)));
    connect(ui->view_ToOther, SIGNAL(triggered()), this, SLOT(setPathToOther()));
    connect(ui->bookmark_Edit, SIGNAL(triggered()), this, SLOT(editBookmark()));
    connect(ui->bookmark_Show, SIGNAL(triggered()), this, SLOT(showBookmarkDialog()));
}

void MainWindow::replaceVars(QString &str, const QFileInfo info)
{
    qDebug() << "MainWindow::replaceVars" << str;

    str.replace("$B", info.completeBaseName());
    str.replace("$E", info.suffix());
    str.replace("$F", info.fileName());
    if (info.isDir()) {
        str.replace("$D", info.absoluteFilePath());
    }
    else {
        str.replace("$D", info.absolutePath());
    }
    str.replace("$P", info.absoluteFilePath());
}

FolderView* MainWindow::otherSideFolderView(const FolderView *view) const
{
    qDebug() << "MainWindow::otherSideFolderView()";

    if (view == ui->pane1->folderPanel()->folderView()) {
        return ui->pane2->folderPanel()->folderView();
    }
    else {
        Q_ASSERT(view == ui->pane2->folderPanel()->folderView());
        return ui->pane1->folderPanel()->folderView();
    }
}

void MainWindow::setViewMode(ModeFlags flags)
{
    qDebug() << "MainWindow::setViewMode();" << flags;
    m_viewMode = flags;
}

bool MainWindow::startProcess(const QString &cmd, const QString &workDir, const QString &errMsg)
{
    qDebug() << "MainWindow::startProcess" << cmd << workDir << errMsg;

    QProcess process(this);
    process.setWorkingDirectory(workDir);
    if (!process.startDetached(cmd)) {
        QMessageBox::critical(this, tr("エラー"), errMsg + "<br/>" + cmd);
        return false;
    }
    return true;
}

void MainWindow::updateActions()
{
    qDebug() << "MainWindow::updateActions" << m_viewMode;

    QWidget *w = qApp->focusWidget();
    FolderView *view;
    if ((view = qobject_cast<FolderView*>(w))) {
        setEnabledAllActions(true);
        qDebug() << ">>>>> フォルダビューの共通メニュー設定 <<<<<";
        ui->action_SearchNext->setEnabled(false);
        ui->action_SearchPrev->setEnabled(false);

        // 「開く」アクションを変更する
        QFileInfo info(view->currentItem());
        if (info.isDir()) {
            ui->action_Open->setIcon(QIcon(":/images/Open.png"));
            ui->action_Open->setText(tr("開く"));
            ui->action_Open->setToolTip(tr("開く"));
        }
        else {
            ui->action_Open->setIcon(QIcon(":/images/Search text.png"));
            ui->action_Open->setText(tr("内蔵ビューアで開く"));
            ui->action_Open->setToolTip(tr("内蔵ビューアで開く"));

            QSettings settings;
            if (!settings.value(IniKey_ViewerForceOpen).toBool()) {
                QStringList list = settings.value(IniKey_ViewerIgnoreExt).toString().split(",");
                foreach (const QString &ext, list) {
                    if (ext.toLower() == info.suffix().toLower()) {
                        ui->action_Open->setEnabled(false);
                        break;
                    }
                }
            }
        }

        if (info.fileName() == "..") {
            // ファイル操作を抑止
            ui->cmd_Copy->setEnabled(false);
            ui->cmd_Delete->setEnabled(false);
            ui->cmd_Move->setEnabled(false);
            ui->cmd_Rename->setEnabled(false);
        }

        if (m_viewMode & ModeBasic) {
            qDebug() << ">>>>> 通常モードのメニュー設定 <<<<<";
            ui->action_SearchNext->setEnabled(false);
            ui->action_SearchPrev->setEnabled(false);

            ui->move_Back->setEnabled(!view->history()->isBegin());
            ui->move_Forward->setEnabled(!view->history()->isEnd());

            QSettings settings;
            ui->action_OpenEditor->setEnabled(!settings.value(IniKey_EditorPath).toString().isEmpty());
            ui->action_OpenTerminal->setEnabled(!settings.value(IniKey_TerminalPath).toString().isEmpty());

        }
        else if (!otherSideFolderView(view)->isVisible()) {
            qDebug() << ">>>>> ハーフモードのメニュー設定 <<<<<";
            ui->view_FromOther->setEnabled(false);
            ui->view_ToOther->setEnabled(false);
            ui->cmd_Copy->setEnabled(false);
            ui->cmd_Move->setEnabled(false);
        }
    }
    else if (qobject_cast<SearchBox*>(w)) {
        qDebug() << ">>>>> 検索モードのメニュー設定 <<<<<";
        setEnabledAllActions(false);
        ui->action_Search->setEnabled(true);
        ui->action_SearchNext->setEnabled(true);
        ui->action_SearchPrev->setEnabled(true);
        ui->help_About->setEnabled(true);
    }
    else if (qobject_cast<SimpleImageView*>(w) ||
             qobject_cast<SimpleTextView*>(w))
    {
        qDebug() << ">>>>> ビューアモードのメニュー設定 <<<<<";
        setEnabledAllActions(false);
        ui->action_Quit->setEnabled(true);
        ui->action_Setting->setEnabled(true);
        ui->check_Update->setEnabled(true);
        ui->view_FontSizeDown->setEnabled(true);
        ui->view_FontSizeUp->setEnabled(true);
        ui->move_Begin->setEnabled(true);
        ui->move_Down->setEnabled(true);
        ui->move_End->setEnabled(true);
        ui->move_Up->setEnabled(true);
        ui->help_About->setEnabled(true);
        ui->key_Left->setEnabled(true);
        ui->key_Right->setEnabled(true);
        if (m_viewMode & ModeHalfView) {
            ui->view_HalfMode->setEnabled(true);
        }
    }
}

void MainWindow::setEnabledAllActions(bool enable)
{
    qDebug() << "MainWindow::setEnabledAllActions();" << enable;
    foreach (QObject *obj, children()) {
        QAction *action = qobject_cast<QAction*>(obj);
        if (action) {
            action->setEnabled(enable);
        }
    }
}

void MainWindow::about()
{
    qDebug() << ">>>>> about <<<<<";

    QMessageBox::about(
                this,
                tr("げふぅ について"),
                tr("<h3>Gefu Ver%1</h3>").arg(VERSION_VALUE) +
                tr("<center>Gefu is an Experimental File Utility.<br/>"
                   "<small>（げふぅは実験的なファイルユーティリティです）</small></center>"
                   "<p>最新版の情報は<a href='https://sourceforge.jp/projects/gefu/'>プロジェクトサイト</a>で公開しています。</p>"
                   "<p><small>Copyright 2014 @miyabi_satoh All rights reserved.</small></p>"));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow::closeEvent();";
    qDebug() << ">>>>> アプリケーションの終了要求 <<<<<";
    QSettings settings;

    if (settings.value(IniKey_ConfirmExit).toBool()) {
        QMessageBox msgBox;
        QCheckBox *checkBox = new QCheckBox();
        checkBox->setText(tr("次回以降は確認しない"));
        msgBox.setCheckBox(checkBox);
        msgBox.setText(tr("終了しますか？"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setIcon(QMessageBox::Question);

        if (msgBox.exec() == QMessageBox::No) {
            qDebug() << ">>>>> ユーザによりキャンセルされました <<<<<";
            event->ignore();
            return;
        }
        settings.setValue(IniKey_ConfirmExit, !checkBox->isChecked());
    }

    settings.setValue(QString("Left/") + IniKey_Dir, ui->pane1->folderPanel()->folderView()->dir());
    settings.setValue(QString("Right/") + IniKey_Dir, ui->pane2->folderPanel()->folderView()->dir());
    settings.setValue(IniKey_ShowHidden, ui->view_Hidden->isChecked());
    settings.setValue(IniKey_ShowSystem, ui->view_System->isChecked());
    settings.setValue(IniKey_WindowGeometry, saveGeometry());
    settings.setValue(iniKey_WindowState, saveState());
    QMainWindow::closeEvent(event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{

    QString ksq = KeyEventToSequence(event);

    qDebug() << ">>>>> キーイベントを受信(MainWindow)" << ksq << "<<<<<";
    qDebug() << qApp->keyboardModifiers().testFlag(Qt::ShiftModifier);

    if (ProcessShortcut(ksq, this)) {
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}
