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

    // ウィンドウタイトルを設定する
    setWindowTitle(tr("げふぅ v0.00"));

    // ウィンドウ初期サイズを設定する
    resize(800, 600);
}

MainWindow::~MainWindow()
{
    delete ui;
}

///
/// \brief MainWindow::onMoveOther
///
/// キーボードフォーカスを他方のパネルに移動します
///
void MainWindow::onMoveOther()
{
    FolderPanel *fp = this->inactivePanel();
    if (fp) {
        fp->fileTable()->setFocus();
    }
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

///
/// \brief MainWindow::on_view_FromOther_triggered
///
/// 隣のパネルと同じフォルダを表示します(O)
void MainWindow::on_view_FromOther_triggered()
{
    this->onViewFromOther();
}
void MainWindow::onViewFromOther()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    fp1->setCurrentFolder(fp2->dir()->absolutePath());
}

///
/// \brief MainWindow::on_view_ToOther_triggered
///
/// 隣のパネルに同じフォルダを表示します(Ctrl + O)
///
void MainWindow::on_view_ToOther_triggered()
{
    FolderPanel *fp1 = activePanel();
    FolderPanel *fp2 = inactivePanel();
    if (fp1 == NULL || fp2 == NULL) {
        return;
    }

    fp2->setCurrentFolder(fp1->dir()->absolutePath());
}

///
/// \brief MainWindow::on_mark_Toggle_triggered
///
/// マークを設定または解除します
///
void MainWindow::on_mark_Toggle_triggered()
{
    this->onMarkToggle();
}
void MainWindow::onMarkToggle()
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

///
/// \brief MainWindow::on_mark_All_triggered
///
/// すべてマークします(Ctrl + A)
///
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

///
/// \brief MainWindow::on_mark_AllFiles_triggered
///
/// すべての「ファイル」をマークします(A)
///
void MainWindow::on_mark_AllFiles_triggered()
{
    this->onMarkAllFiles();
}
void MainWindow::onMarkAllFiles()
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
/// すべてのマークを解除します(Shift + A)
///
void MainWindow::on_mark_AllOff_triggered()
{
    this->onMarkAllOff();
}
void MainWindow::onMarkAllOff()
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

///
/// \brief MainWindow::on_mark_Invert_triggered
///
/// マークを反転します(I)
///
void MainWindow::on_mark_Invert_triggered()
{
    this->onMarkInvert();
}
void MainWindow::onMarkInvert()
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

///
/// \brief MainWindow::on_view_Swap_triggered
///
/// パネルの表示内容を交換します(W)
void MainWindow::on_view_Swap_triggered()
{
    this->onViewSwap();
}
void MainWindow::onViewSwap()
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

///
/// \brief MainWindow::on_move_Down_triggered
///
/// カーソルを下に移動します(J)
///
void MainWindow::on_move_Down_triggered()
{
    this->onMoveDown();
}
void MainWindow::onMoveDown()
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

///
/// \brief MainWindow::on_move_Up_triggered
///
/// カーソルを上に移動します(K)
///
void MainWindow::on_move_Up_triggered()
{
    this->onMoveUp();
}
void MainWindow::onMoveUp()
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

///
/// \brief MainWindow::on_move_Parent_triggered
///
/// 親フォルダに移動します
///
void MainWindow::on_move_Parent_triggered()
{
    this->onMoveParent();
}
void MainWindow::onMoveParent()
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
