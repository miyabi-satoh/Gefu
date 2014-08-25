#include "common.h"
#include "mainwindow.h"
#include "preferencedialog.h"
#include "folderview.h"
#include "searchbox.h"
#include "locationbox.h"
#include "ui_mainwindow.h"
#include "copymoveworker.h"
#include "operationdialog.h"
#include "overwritedialog.h"
#include "deleteworker.h"
#include "renameworker.h"
#include "renamesingledialog.h"
#include "renamemultidialog.h"
#include "historydialog.h"
#include "sortdialog.h"

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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_focusedView(NULL),
    m_overwriteDialog(NULL)
{
    ui->setupUi(this);

    m_overwriteDialog = new OverWriteDialog(this);

    initActionConnections();
    connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), this, SLOT(focusChange(QWidget*,QWidget*)));
    connect(ui->textView, SIGNAL(viewFinished(QWidget*)), this, SLOT(viewFinish(QWidget*)));

    // テキストビューアは初期状態で非表示
    ui->textView->setVisible(false);

    for (int i = 1; i <= 2; i++) {
        LocationBox *locationBox = findChild<LocationBox*>(QString("locationBox%1").arg(i));
        Q_CHECK_PTR(locationBox);

        FolderView *folderView = findChild<FolderView*>(QString("folderView%1").arg(i));
        Q_CHECK_PTR(folderView);

        SearchBox *searchBox = findChild<SearchBox*>(QString("searchBox%1").arg(i));
        Q_CHECK_PTR(searchBox);

        // シグナル＆スロット
        connect(folderView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(open(QModelIndex)));
        connect(folderView, SIGNAL(itemFound(FolderView*)), this, SLOT(itemFound(FolderView*)));
        connect(folderView, SIGNAL(itemNotFound(FolderView*)), this, SLOT(itemNotFound(FolderView*)));
        connect(folderView, SIGNAL(keyPressed(FolderView*,QKeyEvent*)), this, SLOT(keyPress(FolderView*,QKeyEvent*)));
        connect(folderView, SIGNAL(retrieveFinished()), this, SLOT(retrieveFinish()));
        connect(folderView, SIGNAL(retrieveStarted(QString)), this, SLOT(retrieveStart(QString)));
        connect(searchBox, SIGNAL(textEdited(QString)), this, SLOT(searchItem(QString)));
        connect(searchBox, SIGNAL(returnPressed()), this, SLOT(returnPressInSearchBox()));

        // 検索ボックスは初期状態で非表示
        searchBox->setVisible(false);

        // ロケーションボックスを初期化する
        locationBox->initialize();

        // フィルタを初期化する
        setNameFilters(folderView);

        // ソートを初期化する
        setSorting(folderView);

        // フォルダビューを初期化する
        folderView->initialize();
    }

    QSettings settings;
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
    shortcuts = ui->view_Filter->shortcuts();
    shortcuts.append(QKeySequence("Shift+*"));  // マヂで！？
    ui->view_Filter->setShortcuts(shortcuts);

    shortcuts = ui->action_OpenTerminal->shortcuts();
    shortcuts.append(QKeySequence("Shift+>"));  // マヂで！？
    ui->action_OpenTerminal->setShortcuts(shortcuts);

    shortcuts = ui->help_About->shortcuts();
    shortcuts.append(QKeySequence("Shift+?"));  // マヂで！？
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

    ui->folderView1->repaint();
    ui->folderView2->repaint();
}

MainWindow::~MainWindow()
{
    delete ui;
}

FolderView* MainWindow::otherSideView(const FolderView *view) const
{
    qDebug() << "MainWindow::otherSideView()" << view->objectName();

    if (view == ui->folderView1) {
        return ui->folderView2;
    }

    Q_ASSERT(view == ui->folderView2);
    return ui->folderView1;
}

void MainWindow::focusChange(QWidget *old, QWidget *now)
{
    Q_UNUSED(old);
    Q_UNUSED(now);
    qDebug() << "MainWindow::focusChange";

    updateActions();
}

void MainWindow::executeCommand()
{
    qDebug() << "MainWindow::executeCommand";

    FolderView *v = folderView();
    Q_ASSERT(v);

    QFileInfoList list = v->selectedItems();
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
        startProcess(command, v->dir(), tr("コマンドの実行に失敗しました。"));
    }
}

void MainWindow::historyBack()
{
    qDebug() << "MainWindow::historyBack();";

    if (folderView()->historyBack()) {
        updateActions();
    }
}

void MainWindow::historyForward()
{
    qDebug() << "MainWindow::historyForward();";

    if (folderView()->historyForward()) {
        updateActions();
    }
}

void MainWindow::itemFound(FolderView *view)
{
    qDebug() << "MainWindow::itemFound";

    SearchBox *box = searchBox(view);
    QPalette pal = box->palette();
    pal.setColor(QPalette::Text, QPalette().text().color());
    box->setPalette(pal);
}

void MainWindow::itemNotFound(FolderView *view)
{
    qDebug() << "MainWindow::itemNotFound";

    SearchBox *box = searchBox(view);
    QPalette pal = box->palette();
    pal.setColor(QPalette::Text, Qt::red);
    box->setPalette(pal);
}

void MainWindow::markAll()
{
    qDebug() << "MainWindow::markAll();";

    folderView()->setCheckStateAll(true);
}

void MainWindow::markAllFiles()
{
    qDebug() << "MainWindow::markAllFiles();";

    folderView()->setCheckStateAllFiles();
}

void MainWindow::markAllOff()
{
    qDebug() << "MainWindow::markAllOff();";

    folderView()->setCheckStateAll(false);
}

void MainWindow::markInvert()
{
    qDebug() << "MainWindow::markInvert();";

    folderView()->invertCheckState();
}

void MainWindow::markToggle()
{
    qDebug() << "MainWindow::markToggle();";

    folderView()->toggleCheckState(folderView()->currentIndex());
}

void MainWindow::moveItems()
{
    qDebug() << "MainWindow::moveItems";

    FolderView *v = folderView();
    QFileInfoList list = v->selectedItems();
    if (list.isEmpty()) {
        return;
    }

    QSettings settings;
    if (settings.value(IniKey_ConfirmCopy).toBool()) {
        if (QMessageBox::question(this, tr("確認"), tr("移動を実行しますか？"))
            != QMessageBox::Yes)
        {
            return;
        }
    }

    // 上書き確認ダイアログを初期化する
    m_overwriteDialog->reset();

    // ワーカースレッドを作成する
    FolderView *other = otherSideView(v);
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
    opDlg.setAutoClose(settings.value(IniKey_AutoCloseCopy).toBool());

    opDlg.exec();

    settings.setValue(IniKey_AutoCloseCopy, opDlg.autoClose());

    // 念のため、リフレッシュ
    ui->folderView1->refresh();
    ui->folderView2->refresh();
}

void MainWindow::keyPress(FolderView *view, QKeyEvent *event)
{
    QString ksq = KeyEventToSequence(event);
    if (ksq == "") {
        event->ignore();
        return;
    }

    qDebug() << "MainWindow::keyPress" << ksq;

    foreach (QObject *obj, children()) {
        QAction *action = qobject_cast<QAction*>(obj);
        if (!action) {
            continue;
        }

        foreach (const QKeySequence &k, action->shortcuts()) {
            if (ksq == k.toString()) {
                if (action->isEnabled()) {
                    if (action->isCheckable()) {
                        action->setChecked(!action->isChecked());
                    }
                    else {
                        emit action->triggered();
                    }
                    qDebug() << "emit" << action->objectName();
                }
                event->accept();
                return;
            }
        }
    }

    if (ksq == "Left" || ksq == "Right") {
        if (view->side() == ksq) {
            setPathToParent();
        }
        else {
            otherSideView(view)->setFocus();
        }
        event->accept();
        return;
    }

    event->ignore();
}

void MainWindow::returnPressInSearchBox()
{
    qDebug() << "MainWindow::returnPressInSearchBox";

    if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
        emit ui->action_SearchPrev->triggered();
    }
    else {
        emit ui->action_SearchNext->triggered();
    }
}

void MainWindow::chooseFolder()
{
    qDebug() << "MainWindow::chooseFolder();";

    QString path = QFileDialog::getExistingDirectory(
                this, tr("フォルダを選択"), folderView()->dir());
    if (!path.isEmpty()) {
        folderView()->setPath(path, true);
        updateActions();
    }
}

void MainWindow::copyFilenameToClipboard()
{
    qDebug() << "MainWindow::copyFilenameToClipboard();";

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(folderView()->currentItem().fileName());
}

void MainWindow::copyFullpathTpClipboard()
{
    qDebug() << "MainWindow::copyFullpathTpClipboard();";

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(folderView()->currentItem().absoluteFilePath());
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

void MainWindow::copyItems()
{
    qDebug() << "MainWindow::copyItems";

    FolderView *v = folderView();
    QFileInfoList list = v->selectedItems();
    if (list.isEmpty()) {
        return;
    }

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
    FolderView *other = otherSideView(v);
    CopyMoveWorker *worker = new CopyMoveWorker();
    connect(worker, SIGNAL(askOverWrite(QString*,QString*,QString,QString)),
            this, SLOT(askOverWrite(QString*,QString*,QString,QString)));
    worker->setCopyList(&list);
    worker->setTargetDir(other->dir());
    worker->setMoveMode(false);

    // 進捗ダイアログを表示して、処理を開始する
    OperationDialog opDlg(this);
    opDlg.setWindowTitle(tr("コピー"));
    opDlg.setWorker(worker);
    opDlg.setAutoClose(settings.value(IniKey_AutoCloseCopy).toBool());

    opDlg.exec();

    settings.setValue(IniKey_AutoCloseCopy, opDlg.autoClose());

    // 念のため、リフレッシュ
    ui->folderView1->refresh();
    ui->folderView2->refresh();
}

void MainWindow::createFile()
{
    qDebug() << "MainWindow::createFile";

    bool bOk;
    QString name = QInputDialog::getText(
                this, tr("ファイルを作成"), tr("ファイル名："),
                QLineEdit::Normal, "", &bOk);
    if (!bOk || name.isEmpty()) {
        return;
    }

    QDir dir(folderView()->dir());
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
    }
}

void MainWindow::createFolder()
{
    qDebug() << "MainWindow::createFolder";

    bool bOk;
    QString name = QInputDialog::getText(
                this, tr("フォルダを作成"), tr("フォルダ名："),
                QLineEdit::Normal, "", &bOk);
    if (!bOk || name.isEmpty()) {
        return;
    }

    QDir dir(folderView()->dir());
    if (!dir.mkpath(name)) {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("フォルダの作成に失敗しました。"));
    }
    else {
        QSettings settings;
        if (settings.value(IniKey_MoveAfterCreateFolder).toBool()) {
            folderView()->setPath(dir.absoluteFilePath(name), true);
            updateActions();
        }
    }
}

void MainWindow::deleteItems()
{
    qDebug() << "MainWindow::deleteItems";

    FolderView *v = folderView();
    QFileInfoList list = v->selectedItems();
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

    opDlg.exec();

    settings.setValue(IniKey_AutoCloseDelete, opDlg.autoClose());

    // 念のため、リフレッシュ
    ui->folderView1->refresh();
    ui->folderView2->refresh();
}

void MainWindow::open(const QModelIndex &index)
{
    Q_UNUSED(index);
    qDebug() << "MainWindow::open";

    FolderView *v = folderView();
    Q_ASSERT(v);

    QFileInfo info = v->currentItem();
    if (info.isDir()) {
        v->setPath(info.absoluteFilePath(), true);
        updateActions();
        return;
    }

    QSettings settings;
    if (!settings.value(IniKey_ViewerForceOpen).toBool()) {
        QStringList list = settings.value(IniKey_ViewerIgnoreExt).toString().split(",");
        foreach (const QString &ext, list) {
            if (ext.toLower() == info.suffix().toLower()) {
                shellExecute();
                return;
            }
        }
    }

    setUpdatesEnabled(false);

    foreach (QObject *obj, this->children()) {
        QAction *action = qobject_cast<QAction*>(obj);
        if (action) {
            if (action->objectName() == "help_About" ||
                action->objectName() == "check_Update" ||
                action->objectName() == "copy_Fullpath" ||
                action->objectName() == "copy_Filename" ||
                action->objectName() == "action_Quit" ||
                action->objectName() == "action_Setting")
            {
                continue;
            }
            action->setEnabled(false);
        }
    }

    m_focusedView = folderView();
    ui->splitter->setVisible(false);
    ui->textView->setVisible(true);
    ui->textView->setFocus();

    setUpdatesEnabled(true);

    QFile file(info.absoluteFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        ui->textView->setSource(file.readAll());
    }
}

void MainWindow::openEditor(const QString &path)
{
    qDebug() << "MainWindow::openEditor";

    QSettings settings;
    QString exe = settings.value(IniKey_EditorPath).toString();
    if (exe.isEmpty()) {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("外部エディタのパスが未定義です。"));
        return;
    }

    FolderView *v = folderView();
    Q_ASSERT(v);

    QFileInfoList list;
    if (path.isEmpty()) {
        list = v->selectedItems();
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

    QSettings settings;
    QString exe = settings.value(IniKey_TerminalPath).toString();
    if (exe.isEmpty()) {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("ターミナルのパスが未定義です。"));
        return;
    }

    FolderView *v = folderView();
    Q_ASSERT(v);

    foreach (const QFileInfo &info, v->selectedItems()) {
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
    }
}

void MainWindow::refresh()
{
    qDebug() << "MainWindow::refresh();";

    folderView()->refresh();
    updateActions();
}

void MainWindow::renameItems()
{
    qDebug() << "MainWindow::renameItems";

    FolderView *v = folderView();

    QFileInfoList list = v->selectedItems();
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
    dlg->setWorkingDirectory(v->dir());
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

    settings.setValue(IniKey_AutoCloseRename, opDlg.autoClose());
}

void MainWindow::retrieveFinish()
{
    qDebug() << "MainWindow::retrieveFinish();";

    ui->statusBar->showMessage(tr("レディ"), 5000);
}

void MainWindow::retrieveStart(const QString &path)
{
    qDebug() << "MainWindow::retrieveStart();" << path;

    ui->statusBar->showMessage(tr("ファイルリストを取得しています..."));
    if (sender() == ui->folderView1) {
        ui->locationBox1->setText(path);
    }
    else {
        Q_ASSERT(sender() == ui->folderView2);
        ui->locationBox2->setText(path);
    }
}

void MainWindow::shellExecute()
{
    qDebug() << "MainWindow::shellExecute";

    FolderView *v = folderView();
    Q_ASSERT(v);

    QFileInfoList list = v->selectedItems();
    foreach (const QFileInfo &info, list) {
        QString path = QDir::toNativeSeparators(info.absoluteFilePath());
        QDesktopServices::openUrl(QUrl("file:///" + path));
    }
}

void MainWindow::showFilterDialog()
{
    qDebug() << "MainWindow::showFilterDialog();";

    QString filters = folderView()->nameFilters().join(" ");

    QInputDialog dlg(this);
    dlg.setInputMode(QInputDialog::TextInput);
    dlg.setWindowTitle(tr("フィルタを設定"));
    dlg.setLabelText(tr("フィルタ："));
    dlg.setTextValue(filters);
    dlg.resize(width() * 0.8, dlg.height());

    if (dlg.exec() == QDialog::Accepted) {
        setNameFilters(folderView(), dlg.textValue());
    }
}

void MainWindow::showHistoryDialog()
{
    qDebug() << "MainWindow::showHistoryDialog();";

    FolderView *v = folderView();
    FolderView *vOther = otherSideView(v);

    HistoryDialog dlg(this);
    if (v->side() == "Left") {
        dlg.setDefaultLeft(true);
        dlg.setHistory(v->history(), vOther->history());
    }
    else {
        dlg.setDefaultLeft(false);
        dlg.setHistory(vOther->history(), v->history());
    }

    if (dlg.exec() == QDialog::Accepted) {
        if (v->side() == dlg.selectedSide()) {
            v->setHistoryIndexAt(dlg.selectedIndex());
        }
        else {
            v->setPath(vOther->history()->at(dlg.selectedIndex()), true);
        }
        updateActions();
    }
}

void MainWindow::showSortDialog()
{
    qDebug() << "MainWindow::showSortDialog();";

    SortDialog dlg(this);
    dlg.setRightOrLeft(folderView()->side());

    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    setSorting(folderView());
}

void MainWindow::swapView()
{
    qDebug() << "MainWindow::swapView();";

    QString dir1 = ui->folderView1->dir();
    QString dir2 = ui->folderView2->dir();

    ui->folderView1->setPath(dir2, true);
    ui->folderView2->setPath(dir1, true);

    updateActions();
}

void MainWindow::setSorting(FolderView *view)
{
    qDebug() << "MainWindow::setSorting();" << view->objectName();

    QSettings settings;
    QDir::SortFlags flags;

    int sortBy = settings.value(view->side() + slash + IniKey_SortBy).toInt();
    switch (sortBy) {
    case SortByDate:    flags |= QDir::Time; break;
    case SortBySize:    flags |= QDir::Size; break;
    case SortByType:    flags |= QDir::Type; break;
    default:            flags |= QDir::Name; break;
    }

    // デフォルトだと文字列は昇順で、数値は降順…orz
    int orderBy = settings.value(view->side() + slash + IniKey_OrderBy).toInt();
    if (((sortBy == SortByName || sortBy == SortByType) && orderBy == OrderByDesc) ||
        ((sortBy == SortByDate || sortBy == SortBySize) && orderBy == OrderByAsc))
    {
        flags |= QDir::Reversed;
    }

    switch (settings.value(view->side() + slash + IniKey_PutDirs).toInt()) {
    case PutDirsFirst:  flags |= QDir::DirsFirst; break;
    case PutDirsLast:   flags |= QDir::DirsLast; break;
    }

    if (settings.value(view->side() + slash + IniKey_IgnoreCase).toBool()) {
        flags |= QDir::IgnoreCase;
    }

    view->setSorting(flags);
}

void MainWindow::searchItem(const QString &text)
{
    qDebug() << "MainWindow::searchItem" << text;

    FolderView *v;
    SearchBox *box;
    if (ui->searchBox1->hasFocus()) {
        v = ui->folderView1;
        box = ui->searchBox1;
    }
    else {
        Q_ASSERT(ui->searchBox2->hasFocus());
        v = ui->folderView2;
        box = ui->searchBox2;
    }

    if (text.right(1) == "/") {
        box->setText(text.left(text.length() - 1));
        ui->action_Search->setChecked(false);
        return;
    }

    v->searchItem(box->text());
}

void MainWindow::searchNext()
{
    qDebug() << "MainWindow::searchNext";

    FolderView *v = folderView();
    SearchBox *box = searchBox(v);
    v->searchNext(box->text());
}

void MainWindow::searchPrev()
{
    qDebug() << "MainWindow::searchPrev";

    FolderView *v = folderView();
    SearchBox *box = searchBox(v);
    v->searchPrev(box->text());
}

void MainWindow::setCursorToBegin()
{
    qDebug() << "MainWindow::setCursorToBegin();";

    FolderView *v = folderView();
    int row = 0;
    v->setCurrentIndex(v->model()->index(row, 0));
}

void MainWindow::cursorDown()
{
    qDebug() << "MainWindow::cursorDown();";

    FolderView *v = folderView();
    int row = v->currentIndex().row() + 1;
    if (row < v->model()->rowCount()) {
        v->setCurrentIndex(v->model()->index(row, 0));
    }
}

void MainWindow::cursorUp()
{
    qDebug() << "MainWindow::cursorUp();";

    FolderView *v = folderView();
    int row = v->currentIndex().row() - 1;
    if (row >= 0) {
        v->setCurrentIndex(v->model()->index(row, 0));
    }
}

void MainWindow::setCursorToEnd()
{
    qDebug() << "MainWindow::setCursorToEnd();";

    FolderView *v = folderView();
    int row = v->model()->rowCount() - 1;
    v->setCurrentIndex(v->model()->index(row, 0));
}

void MainWindow::setPathFromOther()
{
    qDebug() << "MainWindow::setPathFromOther();";

    FolderView *view = folderView();
    FolderView *other = otherSideView(view);

    view->setPath(other->dir(), true);
    updateActions();
}

void MainWindow::setPathToHome()
{
    qDebug() << "MainWindow::setPathToHome();";

    folderView()->setPath(QDir::homePath(), true);
    updateActions();
}

void MainWindow::setPathToOther()
{
    qDebug() << "MainWindow::setPathToOther();";

    FolderView *view = folderView();
    FolderView *other = otherSideView(view);

    other->setPath(view->dir(), true);
    updateActions();
}

void MainWindow::setPathToParent()
{
    qDebug() << "MainWindow::setPathToParent();";

    QDir dir(folderView()->dir());
    if (!dir.isRoot()) {
        dir.cdUp();
        folderView()->setPath(dir.absolutePath(), true);
        updateActions();
    }
}

void MainWindow::setPathToRoot()
{
    qDebug() << "MainWindow::setPathToRoot();";

    folderView()->setPath(QDir::rootPath(), true);
    updateActions();
}

void MainWindow::toggleSearchBox(bool checked)
{
    qDebug() << "MainWindow::toggleSearchBox" << checked;

    FolderView *v;
    SearchBox *box;

    if (checked) {
        v = folderView();
        if (v == ui->folderView1) {
            box = ui->searchBox1;
        }
        else {
            Q_ASSERT(v == ui->folderView2);
            box = ui->searchBox2;
        }

        box->setVisible(true);
        box->setFocus();
        box->selectAll();
    }
    else {
        if (ui->searchBox1->isVisible()) {
            box = ui->searchBox1;
            v = ui->folderView1;
        }
        else {
            Q_ASSERT(ui->searchBox2->isVisible());
            box = ui->searchBox2;
            v = ui->folderView2;
        }

        if (box->hasFocus()) {
            v->setFocus();
        }
        box->setVisible(false);
    }
}

void MainWindow::openRequest(const QFileInfo &info)
{
    m_focusedView = QApplication::focusWidget();

    QFile file(info.absoluteFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        ui->splitter->setVisible(false);
        ui->textView->setVisible(true);
        ui->textView->setFocus();
        ui->textView->setSource(file.readAll());
    }
    else {
        QMessageBox::critical(
                    this, tr("エラー"),
                    tr("ファイルの読み込みに失敗しました。"));
    }
}

void MainWindow::showPreferenceDialog()
{
    qDebug() << "MainWindow::showPreferenceDialog";

    PreferenceDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        ui->locationBox1->updateAppearance();
        ui->locationBox2->updateAppearance();
        ui->folderView1->updateAppearance();
        ui->folderView2->updateAppearance();

        updateActions();
    }
}

void MainWindow::toggleShowHiddenFiles(bool checked)
{
    qDebug() << "MainWindow::toggleShowHiddenFiles" << checked;

    ui->folderView1->setFilter(QDir::Hidden, checked);
    ui->folderView1->refresh();

    ui->folderView2->setFilter(QDir::Hidden, checked);
    ui->folderView2->refresh();
}

void MainWindow::toggleShowSystemFiles(bool checked)
{
    qDebug() << "MainWindow::toggleShowSystemFiles" << checked;

    ui->folderView1->setFilter(QDir::System, checked);
    ui->folderView1->refresh();

    ui->folderView2->setFilter(QDir::System, checked);
    ui->folderView2->refresh();
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
    manager->get(QNetworkRequest(QUrl("http://miyabi.rakusaba.jp/wp-content/uploads/gefu_latest_win.html")));
#elif defined(Q_OS_MAC)
    manager->get(QNetworkRequest(QUrl("http://miyabi.rakusaba.jp/wp-content/uploads/gefu_latest_mac.html")));
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

void MainWindow::viewFinish(QWidget *sender)
{
    qDebug() << "MainWindow::viewFinish();" << sender->objectName();

    sender->setVisible(false);
    ui->splitter->setVisible(true);
    m_focusedView->setFocus();
}


void MainWindow::initActionConnections()
{
    qDebug() << "MainWindow::initActionConnections";

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
    connect(ui->move_Forward, SIGNAL(triggered()), this, SLOT(historyForward()));
    connect(ui->move_History, SIGNAL(triggered()), this, SLOT(showHistoryDialog()));
    connect(ui->move_Home, SIGNAL(triggered()), this, SLOT(setPathToHome()));
    connect(ui->move_Jump, SIGNAL(triggered()), this, SLOT(chooseFolder()));
    connect(ui->move_Parent, SIGNAL(triggered()), this, SLOT(setPathToParent()));
    connect(ui->move_Root, SIGNAL(triggered()), this, SLOT(setPathToRoot()));
    connect(ui->move_Up, SIGNAL(triggered()), this, SLOT(cursorUp()));
    connect(ui->view_Filter, SIGNAL(triggered()), this, SLOT(showFilterDialog()));
    connect(ui->view_FromOther, SIGNAL(triggered()), this, SLOT(setPathFromOther()));
    connect(ui->view_Hidden, SIGNAL(toggled(bool)), this, SLOT(toggleShowHiddenFiles(bool)));
    connect(ui->view_Refresh, SIGNAL(triggered()), this, SLOT(refresh()));
    connect(ui->view_Sort, SIGNAL(triggered()), this, SLOT(showSortDialog()));
    connect(ui->view_Swap, SIGNAL(triggered()), this, SLOT(swapView()));
    connect(ui->view_System, SIGNAL(toggled(bool)), this, SLOT(toggleShowSystemFiles(bool)));
    connect(ui->view_ToOther, SIGNAL(triggered()), this, SLOT(setPathToOther()));
}

FolderView *MainWindow::folderView() const
{
    qDebug() << "MainWindow::folderView";

    QWidget *w = qApp->focusWidget();
    if (w == ui->folderView1 || w == ui->searchBox1 || w == ui->locationBox1) {
        return ui->folderView1;
    }
    if (w == ui->folderView2 || w == ui->searchBox2 || w == ui->locationBox2) {
        return ui->folderView2;
    }

    qDebug() << w->objectName();
    return NULL;
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

SearchBox *MainWindow::searchBox(FolderView *view) const
{
    qDebug() << "MainWindow::searchBox()" << view->objectName();

    if (view == ui->folderView1) {
        return ui->searchBox1;
    }
    else {
        Q_ASSERT(view == ui->folderView2);
        return ui->searchBox2;
    }
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
    qDebug() << "MainWindow::updateActions";

    if (ui->textView->hasFocus()) {
        // テキストビューア時
        setEnabledAllActions(false);
        ui->action_Quit->setEnabled(true);
        ui->action_Setting->setEnabled(true);
        ui->check_Update->setEnabled(true);
        ui->copy_Filename->setEnabled(true);
        ui->copy_Fullpath->setEnabled(true);
        ui->help_About->setEnabled(true);
    }
    else if (ui->searchBox1->hasFocus() || ui->searchBox2->hasFocus()) {
        // ファイル検索時
        setEnabledAllActions(false);
        ui->action_Search->setEnabled(true);
        ui->action_SearchNext->setEnabled(true);
        ui->action_SearchPrev->setEnabled(true);
        ui->help_About->setEnabled(true);
        ui->action_Quit->setEnabled(true);
    }
    else if (ui->folderView1->hasFocus() || ui->folderView2->hasFocus()) {
        // 通常時
        setEnabledAllActions(true);
        ui->action_SearchNext->setEnabled(false);
        ui->action_SearchPrev->setEnabled(false);

        ui->move_Back->setEnabled(!folderView()->history()->isBegin());
        ui->move_Forward->setEnabled(!folderView()->history()->isEnd());

        QSettings settings;
        ui->action_OpenEditor->setEnabled(!settings.value(IniKey_EditorPath).toString().isEmpty());
        ui->action_OpenTerminal->setEnabled(!settings.value(IniKey_TerminalPath).toString().isEmpty());
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

void MainWindow::setNameFilters(FolderView *view, const QString &filters)
{
    qDebug() << "MainWindow::setNameFilters();" << view->objectName() << filters;

    QStringList list = filters.split(" ", QString::SkipEmptyParts);
    if (list.isEmpty()) {
        list << "*";
    }
    view->setNameFilters(list);
    filterLabel(view)->setText(tr("フィルタ：") + list.join(" "));
}

QLabel *MainWindow::filterLabel(const FolderView *view) const
{
    qDebug() << "MainWindow::filterLabel()" << view->objectName();
    if (view == ui->folderView1) {
        return ui->filterLabel1;
    }
    else {
        Q_ASSERT(view == ui->folderView2);
        return ui->filterLabel2;
    }
}

void MainWindow::about()
{
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
            event->ignore();
            return;
        }
        settings.setValue(IniKey_ConfirmExit, !checkBox->isChecked());
    }

    settings.setValue(QString("Left/") + IniKey_Dir, ui->folderView1->dir());
    settings.setValue(QString("Right/") + IniKey_Dir, ui->folderView2->dir());
    settings.setValue(IniKey_ShowHidden, ui->view_Hidden->isChecked());
    settings.setValue(IniKey_ShowSystem, ui->view_System->isChecked());
    settings.setValue(IniKey_WindowGeometry, saveGeometry());
    settings.setValue(iniKey_WindowState, saveState());
    QMainWindow::closeEvent(event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    QString ksq = KeyEventToSequence(event);
    qDebug() << ksq << " @MainWindow";

    QMainWindow::keyPressEvent(event);
}
