#include "folderpanel.h"
#include "mainwindow.h"
#include "ui_folderpanel.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QDateTime>
#include <QDesktopServices>

FolderPanel::FolderPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderPanel),
    m_dir(),
    m_IconFactory(),
    m_fsWatcher(new QFileSystemWatcher(this)),
    m_bUpdating(false)
{
    ui->setupUi(this);

    // MainWindowのスロットに接続する
    MainWindow *mainWnd = this->mainWindow();
    if (mainWnd) {
        connect(ui->fileTable, SIGNAL(cellDoubleClicked(int,int)), mainWnd, SLOT(onActionOpen()));
    }

    // ヘッダーラベルを設定する
    QStringList labels;
    labels << tr("") << tr("名前") << tr("サイズ") << tr("更新日時");
    ui->fileTable->setHorizontalHeaderLabels(labels);

    // イベントフィルタを設定する
    ui->fileTable->installEventFilter(this);

    // リサイズ時の動作を設定する
    ui->fileTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->fileTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->fileTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->fileTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    // フォルダの要素を表示
    m_dir.setFilter(QDir::NoDot | QDir::AllEntries);
    m_dir.setSorting(QDir::DirsFirst | QDir::Name);
}

FolderPanel::~FolderPanel()
{
    UninstallWatcher();
    delete ui;
}

QTableWidget* FolderPanel::fileTable()
{
    return ui->fileTable;
}

const QTableWidget* FolderPanel::fileTable() const
{
    return ui->fileTable;
}

MainWindow* FolderPanel::mainWindow()
{
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if (w->objectName() == "MainWindow") {
            return static_cast<MainWindow*>(w);
        }
    }
    return NULL;
}

bool FolderPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_A: {
            // A            すべてのファイルをマーク
            // Shift + A    すべてマーク
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    mainWnd->onMarkAll();
                }
                else {
                    mainWnd->onMarkAllFiles();
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_D: {
            // Ctrl + D ファイルを削除
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ControlModifier) {
                    mainWnd->onCmdDelete();
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_E: {
            // E        エディタで開く
            // Ctrl + E ファイルを作成
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ControlModifier) {
                    mainWnd->onCmdNewFile();
                }
                else {
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_G: {
            // G            カーソルを先頭に移動
            // Shift + G    カーソルを末尾に移動
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    mainWnd->onMoveCursorEnd();
                }
                else {
                    mainWnd->onMoveCursorBegin();
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_H: {
            // H            ホームフォルダに移動
            // Shift + H    隠しファイルを表示/非表示
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    mainWnd->onViewHidden();
                }
                else {
                    mainWnd->onMoveHome();
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_I: {
            // I    マークを反転
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onMarkInvert();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_J: {
            // J            カーソルを下に移動
            // Shift + J    フォルダを選択して移動
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    mainWnd->onMoveJump();
                }
                else {
                    mainWnd->onMoveCursorDown();
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_K: {
            // K        カーソルを上に移動
            // Ctrl + K フォルダを作成
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ControlModifier) {
                    mainWnd->onCmdNewFolder();
                }
                else {
                    mainWnd->onMoveCursorUp();
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_O: {
            // O            隣のパネルと同じフォルダを表示
            // Shift + O    隣のパネルに同じフォルダを表示
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    mainWnd->onViewToOther();
                }
                else {
                    mainWnd->onViewFromOther();
                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_Q: {
            // Q    アプリケーションを終了
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onActionQuit();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_R: {
            // R        履歴を表示
            // Ctrl + R 名前を変更
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ControlModifier) {
                    mainWnd->onCmdRename();
                }
                else {

                }
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_U: {
            // U    すべてのマークを解除
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onMarkAllOff();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_W: {
            // W    表示フォルダを交換
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onViewSwap();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_X: {
            // X     コマンドを実行
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onActionCommand();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_Question: {
            // ?    アプリケーション情報を表示
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onHelpAbout();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_Space: {
            // マーク/解除
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onMarkToggle();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_Tab: {
            // 隣のパネルに移動
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                mainWnd->onMoveOther();
            }
            keyEvent->accept();
            return true; }

        case Qt::Key_Backspace: {
            // BS           親フォルダに移動
            // Shift + BS   ルートフォルダに移動
            MainWindow *mainWnd = this->mainWindow();
            if (mainWnd) {
                if (keyEvent->modifiers() & Qt::ShiftModifier) {
                    mainWnd->onMoveRoot();
                }
                else {
                    mainWnd->onMoveParent();
                }
            }
            keyEvent->accept();
            return true; }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void FolderPanel::setCurrentFolder(const QString &path)
{
    mainWindow()->setStatusText(tr("ファイルリストを更新中..."));
    QString curDir = m_dir.absolutePath();
    m_dir.setPath(QDir::cleanPath(path));
    m_dir.canonicalPath();
    QFileInfoList list = m_dir.entryInfoList();

    if (list.empty()) {
        QMessageBox::critical(
                    this,
                    tr("エラー"),
                    tr("フォルダが存在しないか利用できません。"));
        m_dir.setPath(curDir);
        ui->locationField->setText(curDir);
        mainWindow()->setStatusText(tr("レディ"));
        return;
    }

    // フォルダの変更監視
    InstallWatcher();

    m_bUpdating = true;
    ui->fileTable->model()->removeRows(0, ui->fileTable->rowCount());
    for (int i = 0; i < list.size(); i++) {
        QFileInfo info = list.at(i);
        if (info.fileName() == ".." && m_dir.isRoot()) {
            continue;
        }

        int row = ui->fileTable->rowCount();
        ui->fileTable->insertRow(row);

        // ファイル名とアイコン
        QTableWidgetItem *iName = new QTableWidgetItem(info.fileName());
        iName->setFlags(iName->flags() ^ Qt::ItemIsEditable);
        if (info.fileName() == "..") {
            iName->setIcon(QIcon(":/images/Up.png"));
        }
        else {
            iName->setIcon(m_IconFactory.icon(info));
        }
        ui->fileTable->setItem(row, 1, iName);

        // サイズ
        QString str;
        if (info.isDir()) {
            str = tr("<DIR>");
        }
        else if (info.size() >= 1024 * 1024 * 1024) {
            str = tr("%1GB").arg(int(info.size() / (1024 * 1024 * 1024)));
        }
        else if (info.size() >= 1024 * 1024) {
            str = tr("%1MB").arg(int(info.size() / (1024 * 1024)));
        }
        else if (info.size() >= 1024) {
            str = tr("%1KB").arg(int(info.size() / 1024));
        }
        else {
            str = tr("%1B").arg(info.size());
        }
        QTableWidgetItem *iSize = new QTableWidgetItem(str);
        iSize->setFlags(iSize->flags() ^ Qt::ItemIsEditable);
        iSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

        ui->fileTable->setItem(row, 2, iSize);

        // 最終更新日時
        QTableWidgetItem *iDateTime = new QTableWidgetItem(
                    info.lastModified().toString("yy/MM/dd hh:mm"));
        iDateTime->setFlags(iDateTime->flags() ^ Qt::ItemIsEditable);
        ui->fileTable->setItem(row, 3, iDateTime);

        // シグナル処理の関係で、チェックボックスは最後に追加する
        QTableWidgetItem *iCheck = new QTableWidgetItem(tr(""));
        iCheck->setFlags(iCheck->flags() ^ Qt::ItemIsEditable);
        if (info.fileName() != "..") {
            iCheck->setFlags(iCheck->flags() | Qt::ItemIsUserCheckable);
            iCheck->setCheckState(Qt::Unchecked);
        }
        ui->fileTable->setItem(row, 0, iCheck);

    }
    ui->fileTable->selectRow(0);
    ui->fileTable->resizeRowsToContents();

    ui->locationField->setText(m_dir.absolutePath());
    mainWindow()->setStatusText(tr("レディ"));
    m_bUpdating = false;
}

void FolderPanel::InstallWatcher()
{
    UninstallWatcher();

    m_fsWatcher = new QFileSystemWatcher(this);
    m_fsWatcher->addPath(m_dir.absolutePath());
    connect(m_fsWatcher, SIGNAL(directoryChanged(QString)),
            this, SLOT(on_directoryChanged(QString)));
}

void FolderPanel::UninstallWatcher()
{
    if (m_fsWatcher != NULL) {
        delete m_fsWatcher;
    }
    m_fsWatcher = NULL;
}

void FolderPanel::on_fileTable_cellChanged(int row, int column)
{
    if (column == 0) {
        if (ui->fileTable->item(row, 0)->checkState() == Qt::Checked) {
            for (int n = 0; n < 4; n++) {
                ui->fileTable->item(row, n)->setForeground(Qt::red);
                ui->fileTable->item(row, n)->setBackground(Qt::green);
            }
        }
        else {
            for (int n = 0; n < 4; n++) {
                ui->fileTable->item(row, n)->setForeground(Qt::black);
                ui->fileTable->item(row, n)->setBackground(Qt::white);
            }
        }
        ui->fileTable->selectRow(row);
    }
}

void FolderPanel::on_locationField_editingFinished()
{
    ui->locationField->blockSignals(true);

    QString path = ui->locationField->text();
    setCurrentFolder(path);

    ui->locationField->blockSignals(false);
}

void FolderPanel::on_directoryChanged(QString)
{
    int row = ui->fileTable->currentRow();
    this->setCurrentFolder(m_dir.absolutePath());
    if (row >= ui->fileTable->rowCount()) {
        row = ui->fileTable->rowCount() - 1;
    }
    ui->fileTable->selectRow(row);
}

void FolderPanel::on_fileTable_itemSelectionChanged()
{
    if (m_bUpdating) {
        return;
    }

    int row = ui->fileTable->currentRow();
    if (0 <= row && row < ui->fileTable->rowCount()) {
        mainWindow()->setStatusText(ui->fileTable->item(row, 1)->text());
    }
}
