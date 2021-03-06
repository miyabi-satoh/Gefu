#include "common.h"
#include "mainwindow.h"
#include "searchbox.h"
#include "folderpanel.h"
#include "ui_folderpanel.h"

#include <QDebug>
#include <QSettings>
#include <QStatusBar>

FolderPanel::FolderPanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FolderPanel),
    m_mainWnd(NULL)
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
    m_mainWnd = mainWnd;

    // シグナル＆スロット
    connect(ui->searchBox, SIGNAL(textEdited(QString)), this, SLOT(searchItem(QString)));
    connect(ui->folderView, SIGNAL(doubleClicked(QModelIndex)), mainWnd, SLOT(open(QModelIndex)));
    connect(ui->folderView, SIGNAL(dropAccepted(QFileInfoList,QDropEvent*)), mainWnd, SLOT(dropAccept(QFileInfoList,QDropEvent*)));
    connect(ui->folderView, SIGNAL(currentChanged(QFileInfo)), mainWnd, SLOT(currentChange(QFileInfo)));
    connect(ui->folderView, SIGNAL(requestContextMenu(QContextMenuEvent*)), mainWnd, SLOT(showContextMenu(QContextMenuEvent*)));
    connect(ui->folderView, SIGNAL(retrieveStarted(QString)), ui->locationBox, SLOT(setText(QString)));
    connect(ui->folderView, SIGNAL(dataChanged()), this, SLOT(dataChange()));
    connect(ui->folderView, SIGNAL(itemFound()), this, SLOT(itemFound()));
    connect(ui->folderView, SIGNAL(itemNotFound()), this, SLOT(itemNotFound()));
    connect(ui->bookmarkBtn, SIGNAL(clicked()), this, SLOT(addBookmark()));

    // 初期状態では検索ボックスは非表示
    ui->searchBox->setVisible(false);

    // フォルダビューを初期化する
    ui->folderView->initialize(mainWnd);
}

void FolderPanel::updateAppearance(bool darker)
{
    QSettings settings;
    QPalette pal;
    int darkness = 100;
    if (darker) {
        darkness += settings.value(IniKey_Darkness).toInt();
    }

    // ロケーションボックス
    pal = ui->locationBox->palette();
    pal.setColor(QPalette::Base, settings.value(IniKey_BoxColorBg).value<QColor>().darker(darkness));
    pal.setColor(QPalette::Text, settings.value(IniKey_BoxColorFg).value<QColor>().darker(darkness));
    ui->locationBox->setPalette(pal);
    ui->locationBox->setFont(settings.value(IniKey_BoxFont).value<QFont>());

    // モデル
    FileTableModel *model = qobject_cast<FileTableModel*>(ui->folderView->model());
    model->updateAppearance(darkness);

    // ビュー
    pal = ui->folderView->palette();
    pal.setColor(QPalette::Base, settings.value(IniKey_ViewColorBgNormal).value<QColor>().darker(darkness));
    ui->folderView->setPalette(pal);

    // 行の高さを設定する
    QHeaderView *header = ui->folderView->verticalHeader();
    header->setDefaultSectionSize(QFontMetrics(model->font()).height() * 1.5);

    // 列の幅を設定する
    header = ui->folderView->horizontalHeader();
    header->setSectionResizeMode(0, QHeaderView::Fixed);
    header->setSectionResizeMode(1, QHeaderView::Stretch);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);
}

QLineEdit *FolderPanel::locationBox() const
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

void FolderPanel::addBookmark()
{
    qDebug() << "FolderPanel::addBookmark();";

    QSettings settings;
    int i = 0;
    while (!settings.value(IniKey_BookmarkEntryName(i), "").toString().isEmpty()) {
        i++;
    }

    QFileInfo info(ui->folderView->dir());
    QString name = info.fileName();
    if (name.isEmpty()) {
        name = "root";
    }
    settings.setValue(IniKey_BookmarkEntryName(i), name);
    settings.setValue(IniKey_BookmarkEntryPath(i), info.absoluteFilePath());

    m_mainWnd->statusBar()->showMessage(tr("%1をブックマークに追加しました").arg(name));
}

void FolderPanel::itemFound()
{
    qDebug() << "FolderPanel::itemFound";

    QPalette pal = ui->searchBox->palette();
    pal.setColor(QPalette::Text, QPalette().text().color());
    ui->searchBox->setPalette(pal);
}

void FolderPanel::itemNotFound()
{
    qDebug() << "FolderPanel::itemNotFound";

    QPalette pal = ui->searchBox->palette();
    pal.setColor(QPalette::Text, Qt::red);
    ui->searchBox->setPalette(pal);
}

void FolderPanel::searchItem(const QString &text)
{
    qDebug() << "FolderPanel::searchItem" << text;

    if (text.right(1) == "/") {
        // '/'が入力されたら、検索終了
        ui->searchBox->setText(text.left(text.length() - 1));
        ui->folderView->setFocus();
    }
    else {
        ui->folderView->searchItem(ui->searchBox->text());
    }
}

void FolderPanel::focusInEvent(QFocusEvent *)
{
    qDebug() << "FolderPanel::focusInEvent();";

    ui->folderView->setFocus();
}
