#include "common.h"
#include "mainwindow.h"
#include "searchbox.h"
#include "locationbox.h"
#include "folderpanel.h"
#include "ui_folderpanel.h"

#include <QDebug>

FolderPanel::FolderPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderPanel)
{
    ui->setupUi(this);
}

FolderPanel::~FolderPanel()
{
    delete ui;
}

void FolderPanel::initialize(MainWindow *mainWnd)
{
    qDebug() << "FolderPanel::initialize();";

    // シグナル＆スロット
    connect(ui->folderView, SIGNAL(dataChanged()), this, SLOT(dataChange()));
    connect(ui->folderView, SIGNAL(doubleClicked(QModelIndex)), mainWnd, SLOT(open(QModelIndex)));
    connect(ui->folderView, SIGNAL(dropAccepted(QFileInfoList)), mainWnd, SLOT(dropAccept(QFileInfoList)));
    connect(ui->folderView, SIGNAL(currentChanged(QFileInfo)), mainWnd, SLOT(currentChange(QFileInfo)));
    connect(ui->folderView, SIGNAL(itemFound()), mainWnd, SLOT(itemFound()));
    connect(ui->folderView, SIGNAL(itemNotFound()), mainWnd, SLOT(itemNotFound()));
    connect(ui->folderView, SIGNAL(retrieveFinished()), mainWnd, SLOT(retrieveFinish()));
    connect(ui->folderView, SIGNAL(requestContextMenu(QContextMenuEvent*)), mainWnd, SLOT(showContextMenu(QContextMenuEvent*)));
    connect(ui->folderView, SIGNAL(retrieveStarted(QString)), mainWnd, SLOT(retrieveStart(QString)));
    connect(ui->folderView, SIGNAL(retrieveStarted(QString)), ui->locationBox, SLOT(setText(QString)));
    connect(ui->searchBox, SIGNAL(textEdited(QString)), mainWnd, SLOT(searchItem(QString)));
    connect(ui->searchBox, SIGNAL(returnPressed()), mainWnd, SLOT(returnPressInSearchBox()));

    // 初期状態では検索ボックスは非表示
    ui->searchBox->setVisible(false);

    // ロケーションボックスを初期化する
    ui->locationBox->initialize();

    // フォルダビューを初期化する
    ui->folderView->initialize();
}

LocationBox *FolderPanel::locationBox() const
{
    return ui->locationBox;
}

FolderView *FolderPanel::folderView() const
{
    return ui->folderView;
}

SearchBox *FolderPanel::searchBox() const
{
    return ui->searchBox;
}

QLabel *FolderPanel::filterLabel() const
{
    return ui->filterLabel;
}

void FolderPanel::setNameFilters(const QString &filters)
{
    QStringList list = filters.split(" ", QString::SkipEmptyParts);
    if (list.isEmpty()) {
        list << "*";
    }
    ui->folderView->setNameFilters(list);
    showNameFilters();
}

void FolderPanel::showNameFilters()
{
    ui->filterLabel->setText(tr("フィルタ：") + ui->folderView->nameFilters().join(" "));
}

void FolderPanel::dataChange()
{
    qDebug() << "FolderPanel::dataChange();";

    FolderView *view = static_cast<FolderView*>(sender());
    Q_CHECK_PTR(view);

    QFileInfoList list = view->checkedItems();
    if (list.isEmpty()) {
        showNameFilters();
    }
    else {
        int numFolders = 0;
        int numFiles = 0;
        quint64 size = 0;
        foreach (const QFileInfo &info, list) {
            if (info.isDir()) {
                numFolders++;
            }
            else {
                numFiles++;
                size += info.size();
            }
        }

        QString msg = QString::null;
        if (numFolders > 0) {
            msg += tr("%1個のフォルダ ").arg(numFolders);
        }
        if (numFiles > 0) {
            msg += tr("%1個のファイル ").arg(numFiles);
        }

        if (!msg.isEmpty()) {
            msg += tr("を選択 合計%1").arg(FilesizeToString(size));
        }

        ui->filterLabel->setText(msg);
    }
}

void FolderPanel::focusInEvent(QFocusEvent *)
{
    qDebug() << "FolderPanel::focusInEvent();";

    ui->folderView->setFocus();
}
