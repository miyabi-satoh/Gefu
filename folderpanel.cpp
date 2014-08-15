#include "folderpanel.h"
#include "mainwindow.h"
#include "ui_folderpanel.h"
#include <QFileIconProvider>
#include <QCheckBox>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QDateTime>
#include <QDesktopServices>

FolderPanel::FolderPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderPanel)
{
    ui->setupUi(this);

    // MainWindowのスロットに接続する
    foreach (QWidget *w, qApp->topLevelWidgets()) {
        if (w->objectName() == "MainWindow") {
            MainWindow *mainWnd = static_cast<MainWindow*>(w);
            connect(ui->fileTable, SIGNAL(cellDoubleClicked(int,int)), mainWnd, SLOT(on_action_Open_triggered()));
        }
    }





    // ヘッダーラベルを設定する
    QStringList labels;
    labels << tr("") << tr("Name") << tr("Size") << tr("Date");
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
    setCurrentFolder(QDir::homePath());
    ui->fileTable->resizeColumnsToContents();
}

FolderPanel::~FolderPanel()
{
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

bool FolderPanel::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
//        case Qt::Key_Space: {
//            // 選択アイテムのチェック状態を反転する
//            QModelIndex index = ui->fileTable->currentIndex();
//            int row = index.row();
//            if (ui->fileTable->item(row, 1)->text() == "..") {
//                // 何もしない
//            }
//            else if (ui->fileTable->item(row, 0)->checkState() == Qt::Checked) {
//                ui->fileTable->item(row, 0)->setCheckState(Qt::Unchecked);
//            }
//            else {
//                ui->fileTable->item(row, 0)->setCheckState(Qt::Checked);
//            }
//            // 最終行でなければ、次のアイテムに移動する
//            if (row < ui->fileTable->rowCount() - 1) {
//                QModelIndex nextIndex = ui->fileTable->model()->index(row + 1, 1);
//                ui->fileTable->setCurrentIndex(nextIndex);
//            }
//            keyEvent->accept();
//            return true; }

        case Qt::Key_Tab:
            // MainWindowで処理を行う
            foreach (QWidget *w, qApp->topLevelWidgets()) {
                if (w->objectName() == "MainWindow") {
                    return w->eventFilter(obj, event);
                }
            }
            break;

//        case Qt::Key_Return: {
//            if (keyEvent->modifiers() & Qt::ControlModifier) {
//                int row = ui->fileTable->currentIndex().row();
//                QString path = ui->fileTable->item(row, 1)->text();
//                path = m_dir.absoluteFilePath(path);
//                path = QDir::toNativeSeparators(path);
//                QDesktopServices::openUrl(QUrl("file:///" + path));
//            }
//            keyEvent->accept();
//            return true; }

//        case Qt::Key_Backspace: {
//            if (!m_dir.isRoot()) {
//                QString path = m_dir.absoluteFilePath("..");
//                setCurrentFolder(path);
//            }
//            keyEvent->accept();
//            return true; }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void FolderPanel::setCurrentFolder(const QString &path)
{
    m_dir.setPath(QDir::cleanPath(path));
    QFileInfoList list = m_dir.entryInfoList();

    if (list.empty()) {
        QMessageBox::critical(
                    this,
                    tr("エラー"),
                    tr("フォルダが存在しないか利用できません。"));
        return;
    }

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
            iName->setIcon(QFileIconProvider().icon(info));
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

//void FolderPanel::on_fileTable_doubleClicked(const QModelIndex &index)
//{
//    QString strName = ui->fileTable->item(index.row(), 1)->text();
//    QString strPath = m_dir.absoluteFilePath(strName);
//    QFileInfo info(strPath);

//    if (info.isDir()) {
//        setCurrentFolder(strPath);
//    }
//    else {
//        // TODO:ファイルの場合はどうしよう？
//    }
//}

void FolderPanel::on_locationField_editingFinished()
{
    ui->locationField->blockSignals(true);

    QString path = ui->locationField->text();
    setCurrentFolder(path);

    ui->locationField->blockSignals(false);
}
