#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileSystemModel>
#include <QDebug>
#include <QKeyEvent>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // イベントフィルタを設定する
    this->installEventFilter(this);

    // ウィンドウタイトルを設定する
    setWindowTitle(tr("げふぅ v0.00"));

    // ウィンドウ初期サイズを設定する
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        switch (keyEvent->key()) {
        case Qt::Key_Tab: {
            FolderPanel *fPanel;
            if (obj->parent()->objectName() == "folderPanel_L") {
                fPanel = ui->folderPanel_R;
            }
            else if (obj->parent()->objectName() == "folderPanel_R") {
                fPanel = ui->folderPanel_L;
            }
            else {
                break;
            }
            fPanel->fileTable()->setFocus();
            keyEvent->accept();
            return true; }
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

FolderPanel* MainWindow::activePanel()
{
    if (ui->folderPanel_L->fileTable()->hasFocus()) {
        return ui->folderPanel_L;
    }
    if (ui->folderPanel_R->fileTable()->hasFocus()) {
        return ui->folderPanel_R;
    }

    return NULL;
}

FolderPanel* MainWindow::inactivePanel()
{
    FolderPanel *fp = activePanel();
    if (fp == ui->folderPanel_L) {
        return ui->folderPanel_R;
    }
    if (fp == ui->folderPanel_R) {
        return ui->folderPanel_L;
    }

    return NULL;
}

void MainWindow::on_action_Quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_Open_triggered()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    int row = fp->fileTable()->currentIndex().row();
    QString path = fp->fileTable()->item(row, 1)->text();
    path = fp->dir()->absoluteFilePath(path);
    QFileInfo info(path);

    if (info.isDir()) {
        fp->setCurrentFolder(path);
    }
}

void MainWindow::on_action_Exec_triggered()
{
    FolderPanel *fp = activePanel();
    if (!fp) {
        return;
    }

    int row = fp->fileTable()->currentIndex().row();
    QString path = fp->fileTable()->item(row, 1)->text();
    path = fp->dir()->absoluteFilePath(path);
    path = QDir::toNativeSeparators(path);
    QDesktopServices::openUrl(QUrl("file:///" + path));
}

void MainWindow::on_view_FromOther_triggered()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    fp1->setCurrentFolder(fp2->dir()->absolutePath());
}


void MainWindow::on_view_ToOther_triggered()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    fp2->setCurrentFolder(fp1->dir()->absolutePath());
}

void MainWindow::on_mark_Toggle_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = fp->fileTable()->currentIndex().row();
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        // 何もしない
    }
    else if (fp->fileTable()->item(row, 0)->checkState() == Qt::Checked) {
        fp->fileTable()->item(row, 0)->setCheckState(Qt::Unchecked);
    }
    else {
        fp->fileTable()->item(row, 0)->setCheckState(Qt::Checked);
    }
    // 最終行でなければ、次のアイテムに移動する
    if (row < fp->fileTable()->rowCount() - 1) {
        QModelIndex nextIndex = fp->fileTable()->model()->index(row + 1, 1);
        fp->fileTable()->setCurrentIndex(nextIndex);
    }
}

void MainWindow::on_mark_All_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        fp->fileTable()->item(row, 0)->setCheckState(Qt::Checked);
    }
}

void MainWindow::on_mark_AllFiles_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        QString path = fp->fileTable()->item(row, 1)->text();
        path = fp->dir()->absoluteFilePath(path);

        QFileInfo info(path);
        if (info.isDir()) {
            fp->fileTable()->item(row, 0)->setCheckState(Qt::Unchecked);
        }
        else {
           fp->fileTable()->item(row, 0)->setCheckState(Qt::Checked);
        }
    }
}

///
/// \brief MainWindow::on_mark_AllOff_triggered
///
/// すべてのマークを解除します
///
void MainWindow::on_mark_AllOff_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        fp->fileTable()->item(row, 0)->setCheckState(Qt::Unchecked);
    }
}

void MainWindow::on_mark_Invert_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = 0;
    if (fp->fileTable()->item(row, 1)->text() == "..") {
        row++;
    }

    for (; row < fp->fileTable()->rowCount(); row++) {
        QTableWidgetItem *item = fp->fileTable()->item(row, 0);
        if (item->checkState() == Qt::Checked) {
            item->setCheckState(Qt::Unchecked);
        }
        else {
            item->setCheckState(Qt::Checked);
        }
    }
}

void MainWindow::on_view_Swap_triggered()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    QString path1 = fp1->dir()->absolutePath();
    QString path2 = fp2->dir()->absolutePath();

    fp1->setCurrentFolder(path2);
    fp2->setCurrentFolder(path1);
}

void MainWindow::on_move_Down_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = fp->fileTable()->currentIndex().row();
    if (row < fp->fileTable()->rowCount() - 1) {
        QModelIndex nextIndex = fp->fileTable()->model()->index(row + 1, 1);
        fp->fileTable()->setCurrentIndex(nextIndex);
    }
}

void MainWindow::on_move_Up_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    int row = fp->fileTable()->currentIndex().row();
    if (row > 0) {
        QModelIndex nextIndex = fp->fileTable()->model()->index(row - 1, 1);
        fp->fileTable()->setCurrentIndex(nextIndex);
    }
}

void MainWindow::on_move_Parent_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    if (!fp->dir()->isRoot()) {
        QString path = fp->dir()->absoluteFilePath("..");
        fp->setCurrentFolder(path);
    }
}

void MainWindow::on_move_Root_triggered()
{
    FolderPanel *fp = activePanel();
    if (fp == NULL) {
        return;
    }

    if (!fp->dir()->isRoot()) {
        fp->setCurrentFolder(QDir::rootPath());
    }
}
