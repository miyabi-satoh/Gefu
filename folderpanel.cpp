#include "common.h"
#include "filetablemodel.h"
#include "folderpanel.h"
#include "mainwindow.h"
#include "ui_folderpanel.h"

#include <QSettings>

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
    // 行の高さを75%に調整
    QHeaderView *vHeader = ui->fileTable->verticalHeader();
    vHeader->setDefaultSectionSize(vHeader->defaultSectionSize() * 0.75);
}

FolderPanel::~FolderPanel()
{
    delete ui;
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
    connect(ui->fileTable, SIGNAL(filterChanged()),
            this, SLOT(onFilterChanged()));

    //>>>>> フィルタ初期化
    model->setFilter(QDir::NoDot | QDir::AllDirs | QDir::Files);
    if (settings.value(IniKey_ShowHidden).toBool()) {
        model->setFilter(model->filter() | QDir::Hidden);
    }
    if (settings.value(IniKey_ShowSystem).toBool()) {
        model->setFilter(model->filter() | QDir::System);
    }
    //>>>>> ソート順初期化
    model->setSorting(QDir::Name);  // 0
    int sortBy = settings.value(side + slash + IniKey_SortBy).toInt();
    switch (sortBy) {
    case SortByDate:    model->setSorting(model->sorting() | QDir::Time); break;
    case SortBySize:    model->setSorting(model->sorting() | QDir::Size); break;
    case SortByType:    model->setSorting(model->sorting() | QDir::Type); break;
    default:            model->setSorting(model->sorting() | QDir::Name); break;
    }
    // デフォルトだと文字列は昇順で、数値は降順…orz
    int orderBy = settings.value(side + slash + IniKey_OrderBy).toInt();
    if (((sortBy == SortByName || sortBy == SortByType) && orderBy == OrderByDesc) ||
        ((sortBy == SortByDate || sortBy == SortBySize) && orderBy == OrderByAsc))
    {
        model->setSorting(model->sorting() | QDir::Reversed);
    }
    // フォルダの位置
    switch (settings.value(side + slash + IniKey_PutDirs).toInt()) {
    case PutDirsFirst:  model->setSorting(model->sorting() | QDir::DirsFirst); break;
    case PutDirsLast:   model->setSorting(model->sorting() | QDir::DirsLast); break;
    }
    // 大文字小文字の区別
    if (settings.value(side + slash + IniKey_IgnoreCase).toBool()) {
        model->setSorting(model->sorting() | QDir::IgnoreCase);
    }
    //>>>>> 監視フォルダ初期化
    QString path = settings.value(side + slash + IniKey_Dir).toString();

    model->updateAppearance();
    ui->fileTable->setModel(model);
    ui->fileTable->setRootPath(path, true);
    onFilterChanged();
}

void FolderPanel::updateAppearance()
{
    QSettings settings;
    QPalette palette;
    QFont font;

    font = settings.value(IniKey_BoxFont).value<QFont>();
    palette = ui->locationField->palette();
    palette.setColor(QPalette::Base,
                     settings.value(IniKey_BoxColorBg).value<QColor>());
    palette.setColor(QPalette::Text,
                     settings.value(IniKey_BoxColorFg).value<QColor>());
    ui->locationField->setFont(font);
    ui->locationField->setPalette(palette);

    palette = ui->fileTable->palette();
    palette.setColor(QPalette::Base,
                     settings.value(IniKey_ViewColorBgNormal).value<QColor>());
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

    if (msg.isEmpty()) {
        onFilterChanged();
    }
    else {
        ui->label->setText(msg);
    }
}

void FolderPanel::onFilterChanged()
{
    FileTableModel *m = static_cast<FileTableModel*>(ui->fileTable->model());
    QString filters = "フィルタ：";
    foreach (const QString &filter, m->nameFilters()) {
        filters += filter + " ";
    }
    ui->label->setText(filters);
}

void FolderPanel::on_locationField_editingFinished()
{
    ui->locationField->blockSignals(true);

    QString path = ui->locationField->text();
    ui->fileTable->setRootPath(path, true);

    ui->locationField->blockSignals(false);
}
