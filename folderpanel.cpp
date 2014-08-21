#include "common.h"
#include "filetablemodel.h"
#include "folderpanel.h"
#include "mainwindow.h"
#include "ui_folderpanel.h"
#include <QCheckBox>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>
#include <QDateTime>
#include <QDesktopServices>
#include <QSettings>
#ifdef Q_OS_WIN32
    #include <windows.h>
#endif

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
    ui(new Ui::FolderPanel)
{
    ui->setupUi(this);
    ui->fileTable->setModel(new FileTableModel(this));

    // リサイズ時の動作を設定する
    QHeaderView *header = ui->fileTable->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setDefaultSectionSize(header->minimumSectionSize());

    QHeaderView *vHeader = ui->fileTable->verticalHeader();
    vHeader->setDefaultSectionSize(vHeader->defaultSectionSize() * 0.75);
}

FolderPanel::~FolderPanel()
{
    delete ui;
}

QTableView* FolderPanel::fileTable()
{
    return ui->fileTable;
}

const QTableView *FolderPanel::fileTable() const
{
    return ui->fileTable;
}

const QString FolderPanel::side() const
{
    return ui->fileTable->side();
}

void FolderPanel::setSide(const QString &side)
{
    ui->fileTable->setSide(side);

    QSettings settings;
    FileTableModel *model = new FileTableModel();
    connect(model, SIGNAL(rootChanged(QString)),
            ui->locationField, SLOT(setText(QString)));
    connect(model, SIGNAL(stateChanged(int,int,quint64)),
            this, SLOT(onStateChanged(int,int,quint64)));
    connect(model, SIGNAL(listUpdated()),
            ui->fileTable, SLOT(refresh()));

    //>>>>> フィルタ初期化
    model->setFilter(QDir::NoDot | QDir::AllDirs | QDir::Files);
    if (settings.value(IniKey_ShowHidden, false).toBool()) {
        model->setFilter(model->filter() | QDir::Hidden);
    }
    if (settings.value(IniKey_ShowSystem, false).toBool()) {
        model->setFilter(model->filter() | QDir::System);
    }
    //>>>>> ソート順初期化
    model->setSorting(QDir::Name);  // 0
    int sortBy = settings.value(side + slash + IniKey_SortBy, SortByName).toInt();
    switch (sortBy) {
    case SortByDate:    model->setSorting(model->sorting() | QDir::Time); break;
    case SortBySize:    model->setSorting(model->sorting() | QDir::Size); break;
    case SortByType:    model->setSorting(model->sorting() | QDir::Type); break;
    default:            model->setSorting(model->sorting() | QDir::Name); break;
    }
    // デフォルトだと文字列は昇順で、数値は降順…orz
    int orderBy = settings.value(side + slash + IniKey_OrderBy, OrderByAsc).toInt();
    if (((sortBy == SortByName || sortBy == SortByType) && orderBy == OrderByDesc) ||
        ((sortBy == SortByDate || sortBy == SortBySize) && orderBy == OrderByAsc))
    {
        model->setSorting(model->sorting() | QDir::Reversed);
    }
    // フォルダの位置
    switch (settings.value(side + slash + IniKey_PutDirs, PutDirsFirst).toInt()) {
    case PutDirsFirst:  model->setSorting(model->sorting() | QDir::DirsFirst); break;
    case PutDirsLast:   model->setSorting(model->sorting() | QDir::DirsLast); break;
    }
    // 大文字小文字の区別
    if (settings.value(side + slash + IniKey_IgnoreCase, true).toBool()) {
        model->setSorting(model->sorting() | QDir::IgnoreCase);
    }
    //>>>>> 監視フォルダ初期化
    QString key = side + slash + IniKey_Dir;
    QString path = settings.value(key, QDir::homePath()).toString();

    model->updateAppearance();
    ui->fileTable->setModel(model);
    ui->fileTable->setRootPath(path, true);
}

void FolderPanel::updateAppearance()
{
    QSettings settings;
    QPalette palette;
    QFont font;

    font = ui->locationField->font();
    font = settings.value(IniKey_BoxFont, font).value<QFont>();
    palette = ui->locationField->palette();
    palette.setColor(
                QPalette::Base,
                settings.value(IniKey_BoxColorBg, palette.base()).value<QColor>());
    palette.setColor(
                QPalette::Text,
                settings.value(IniKey_BoxColorFg, palette.text()).value<QColor>());
    ui->locationField->setFont(font);
    ui->locationField->setPalette(palette);

    palette = ui->fileTable->palette();
    palette.setColor(
                QPalette::Base,
                settings.value(IniKey_ViewColorBgNormal, palette.base()).value<QColor>());
    ui->fileTable->setPalette(palette);

    FileTableModel *model = static_cast<FileTableModel*>(ui->fileTable->model());
    if (model) {
        model->updateAppearance();
    }
}

void FolderPanel::onStateChanged(int checkedFolders, int checkedFiles, quint64 totalSize)
{
    QString msg = "";
    if (checkedFolders > 0) {
        msg += tr("%1個のフォルダ ").arg(checkedFolders);
    }
    if (checkedFiles > 0) {
        msg += tr("%1個のファイル ").arg(checkedFiles);
    }
    if (msg.length() > 0) {
        msg += "を選択 合計 ";
        msg += FilesizeToString(totalSize);
    }

    ui->label->setText(msg);

}

void FolderPanel::on_locationField_editingFinished()
{
    ui->locationField->blockSignals(true);

    QString path = ui->locationField->text();
    ui->fileTable->setRootPath(path, true);

    ui->locationField->blockSignals(false);
}

