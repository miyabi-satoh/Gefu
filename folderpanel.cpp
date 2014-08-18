#include "common.h"
#include "folderpanel.h"
#include "mainwindow.h"
#include "ui_folderpanel.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QDateTime>
#include <QDesktopServices>

QString FilesizeToString(quint64 size)
{
    if (size >= 1024 * 1024 * 1024) {
        return QString("%1GB").arg(int(10 * size / (1024 * 1024 * 1024)) / 10.0);
    }
    if (size >= 1024 * 1024) {
        return QString("%1MB").arg(int(10 * size / (1024 * 1024)) / 10.0);
    }
    if (size >= 1024) {
        return QString("%1KB").arg(int(10 * size / 1024) / 10.0);
    }
    return QString("%1B").arg(size);
}

FolderPanel::FolderPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderPanel),
    m_dir(),
    m_IconFactory(),
    m_fsWatcher(new QFileSystemWatcher(this)),
    m_bUpdating(false)
{
    ui->setupUi(this);

    // セル(チェックボックス)の変更
    connect(ui->fileTable, SIGNAL(cellChanged(int,int)),
            this, SLOT(onUpdateMark(int,int)));

    // リサイズ時の動作を設定する
    QHeaderView *header = ui->fileTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    // フォルダの要素を表示
    m_dir.setFilter(QDir::NoDot | QDir::AllEntries);
    m_dir.setSorting(QDir::DirsFirst | QDir::Name);
}

FolderPanel::~FolderPanel()
{
    UninstallWatcher();
    delete ui;
}

FileTableWidget* FolderPanel::fileTable()
{
    return ui->fileTable;
}

const FileTableWidget *FolderPanel::fileTable() const
{
    return ui->fileTable;
}

void FolderPanel::setCurrentFolder(const QString &path)
{
    getMainWnd()->setStatusText(tr("ファイルリストを更新中..."));
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
        getMainWnd()->setStatusText(tr("レディ"));
        return;
    }

    // フォルダの変更監視
    InstallWatcher();

    beginUpdate();
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
        else {
            str = FilesizeToString(info.size());
        }
#if 0
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
#endif
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
    getMainWnd()->setStatusText(tr("レディ"));
    endUpdate();
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

void FolderPanel::onUpdateMark(int, int column)
{
    if (column != 0 || isUpdating()) {
        return;
    }

    // マークフォルダ数、ファイル数、サイズを計算する
    int numFolders = 0;
    int numFiles = 0;
    quint64 sizeTotal = 0;
    for (int n = 0; n < ui->fileTable->rowCount(); n++) {
        if (ui->fileTable->item(n, 0)->checkState() != Qt::Checked) {
            continue;
        }
        QString name = ui->fileTable->item(n, 1)->text();
        QFileInfo info(m_dir.absoluteFilePath(name));
        if (info.isDir()) {
            numFolders++;
        }
        else {
            numFiles++;
            sizeTotal += info.size();
        }
    }

    QString msg = "";
    if (numFolders > 0) {
        msg += tr("%1個のフォルダ ").arg(numFolders);
    }
    if (numFiles > 0) {
        msg += tr("%1個のファイル ").arg(numFiles);
    }
    if (msg.length() > 0) {
        msg += "を選択 合計 ";
        msg += FilesizeToString(sizeTotal);
    }

    ui->label->setText(msg);
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
    if (isUpdating()) {
        return;
    }

    int row = ui->fileTable->currentRow();
    if (0 <= row && row < ui->fileTable->rowCount()) {
        getMainWnd()->setStatusText(ui->fileTable->item(row, 1)->text());
    }
}
