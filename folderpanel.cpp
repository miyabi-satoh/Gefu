#include "folderpanel.h"
#include "ui_folderpanel.h"

FolderPanel::FolderPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderPanel)
{
    ui->setupUi(this);
    // 初期状態では検索ボックスは非表示
    ui->searchBox->setVisible(false);
}

FolderPanel::~FolderPanel()
{
    delete ui;
}

LocationBox *FolderPanel::locationBox() const
{
    return ui->locationBox;
}

FolderView *FolderPanel::folderView() const
{
    return ui->folderView;
}

SearchBox *FolderPanel::serachBox() const
{
    return ui->searchBox;
}

QLabel *FolderPanel::filterLabel() const
{
    return ui->filterLabel;
}


void FolderPanel::focusInEvent(QFocusEvent *)
{
    ui->folderView->setFocus();
}
