#include "common.h"
#include "folderview.h"
#include "anyview.h"
#include "ui_anyview.h"

#include <QDebug>
#include <QSettings>

AnyView::AnyView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnyView)
{
    ui->setupUi(this);

    // 初期状態では全て非表示
    ui->folderPanel->setVisible(false);
    ui->graphicsView->setVisible(false);
    ui->textView->setVisible(false);
}

AnyView::~AnyView()
{
    delete ui;
}

void AnyView::changeView(int viewType)
{
    qDebug() << "AnyView::changeView();" << viewType;

    // 一度すべてを非表示にする
    ui->folderPanel->setVisible(false);
    ui->graphicsView->setVisible(false);
    ui->textView->setVisible(false);

    QWidget *w;
    switch (viewType) {
    case AnyView::ViewFolder:   w = ui->folderPanel; break;
    case AnyView::ViewImage:    w = ui->graphicsView; break;
    case AnyView::ViewText:     w = ui->textView; break;
    default: Q_ASSERT(false);   w = NULL; break;
    }

    this->setVisible(true);
    w->setVisible(true);
}

bool AnyView::setViewItem(const QFileInfo &info)
{
    // フォルダ
    if (info.isDir()) {
        changeView(ViewFolder);
        ui->folderPanel->folderView()->setPath(info.absoluteFilePath(), true);
        return true;
    }

    // 画像ファイル
    changeView(ViewImage);
    if (ui->graphicsView->setSource(info.absoluteFilePath())) {
        return true;
    }

    QSettings settings;
    bool isBinary = false;
    if (!settings.value(IniKey_ViewerForceOpen).toBool()) {
        QStringList list = settings.value(IniKey_ViewerIgnoreExt).toString().split(",");
        foreach (const QString &ext, list) {
            if (ext.toLower() == info.suffix().toLower()) {
                isBinary = true;
                break;
            }
        }
    }

    if (!isBinary) {
        changeView(ViewText);
        QFile file(info.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly)) {
            ui->textView->setSource(file.readAll());
            file.close();
            return true;
        }
    }

    changeView(ViewText);
    ui->textView->setSource("");
    return false;
}

FolderPanel *AnyView::folderPanel() const
{
    return ui->folderPanel;
}

SimpleImageView *AnyView::imageView() const
{
    return ui->graphicsView;
}

SimpleTextView *AnyView::textView() const
{
    return ui->textView;
}


void AnyView::focusInEvent(QFocusEvent *)
{
    qDebug() << "AnyView::focusInEvent();";

    if (ui->folderPanel->isVisible()) {
        ui->folderPanel->folderView()->setFocus();
    }
    else if (ui->graphicsView->isVisible()) {
        ui->graphicsView->setFocus();
    }
    else {
        Q_ASSERT(ui->textView->isVisible());
        ui->textView->setFocus();
    }
}
