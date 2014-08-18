#include "common.h"
#include "mainwindow.h"
#include "filetablewidget.h"
#include <QKeyEvent>
#include <QHeaderView>

FileTableWidget::FileTableWidget(QWidget *parent) :
    QTableWidget(parent)
{
    connect(this, SIGNAL(cellChanged(int,int)), this, SLOT(onCellChanged(int,int)));

    // ダブルクリック → 開く
    MainWindow *mainWnd = getMainWnd();
    connect(this, SIGNAL(cellDoubleClicked(int,int)), mainWnd, SLOT(onActionOpen()));
}

void FileTableWidget::keyPressEvent(QKeyEvent *event)
{
    bool accepted = false;
    switch (event->key()) {
    case Qt::Key_A:
        // A            すべてのファイルをマーク
        // Shift + A    すべてマーク
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMarkAllFiles();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onMarkAll();
            accepted = true;
        }
        break;

    case Qt::Key_C:
        // Ctrl + C ファイルをコピー
        if (event->modifiers() == Qt::ControlModifier) {
            getMainWnd()->onCmdCopy();
            accepted = true;
        }
        break;

    case Qt::Key_D:
        // Ctrl + D ファイルを削除
        if (event->modifiers() == Qt::ControlModifier) {
            getMainWnd()->onCmdDelete();
            accepted = true;
        }
        break;

    case Qt::Key_E:
        // E        エディタで開く
        // Ctrl + E ファイルを作成
        if (event->modifiers() == Qt::NoModifier) {
        }
        else if (event->modifiers() == Qt::ControlModifier) {
            getMainWnd()->onCmdNewFile();
            accepted = true;
        }
        break;

    case Qt::Key_G:
        // G            カーソルを先頭に移動
        // Shift + G    カーソルを末尾に移動
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMoveCursorBegin();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onMoveCursorEnd();
            accepted = true;
        }
        break;

    case Qt::Key_H:
        // H            ホームフォルダに移動
        // Shift + H    隠しファイルを表示/非表示
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMoveHome();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onViewHidden();
            accepted = true;
        }
        break;

    case Qt::Key_I:
        // I    マークを反転
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMarkInvert();
            accepted = true;
        }
        break;

    case Qt::Key_J:
        // J            カーソルを下に移動
        // Shift + J    フォルダを選択して移動
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMoveCursorDown();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onMoveJump();
            accepted = true;
        }
        break;

    case Qt::Key_K:
        // K        カーソルを上に移動
        // Ctrl + K フォルダを作成
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMoveCursorUp();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ControlModifier) {
            getMainWnd()->onCmdNewFolder();
            accepted = true;
        }
        break;

    case Qt::Key_M:
        // M            開く
        // Shift + M    アプリケーションで開く
        // Ctrl + M     ファイルの移動
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onActionOpen();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onActionExec();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ControlModifier) {
            getMainWnd()->onCmdMove();
            accepted = true;
        }
        break;

    case Qt::Key_O:
        // O            隣のパネルと同じフォルダを表示
        // Shift + O    隣のパネルに同じフォルダを表示
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onViewFromOther();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onViewToOther();
            accepted = true;
        }
        break;

    case Qt::Key_Q:
        // Q    アプリケーションを終了
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onActionQuit();
            accepted = true;
        }
        break;

    case Qt::Key_R:
        // R        履歴を表示
        // Ctrl + R 名前を変更
        if (event->modifiers() == Qt::NoModifier) {
        }
        else if (event->modifiers() == Qt::ControlModifier) {
            getMainWnd()->onCmdRename();
            accepted = true;
        }
        break;

    case Qt::Key_S:
        // S            ソート方法を選択
        // Shift + S    システムファイルを表示/非表示
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onViewSort();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onViewSystem();
            accepted = true;
        }
        break;

    case Qt::Key_U:
        // U    すべてのマークを解除
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMarkAllOff();
            accepted = true;
        }
        break;

    case Qt::Key_W:
        // W    表示フォルダを交換
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onViewSwap();
            accepted = true;
        }
        break;

    case Qt::Key_X:
        // X     コマンドを実行
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onActionCommand();
            accepted = true;
        }
        break;

    case Qt::Key_Question:
        // ?    アプリケーション情報を表示
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onHelpAbout();
            accepted = true;
        }
        break;

    case Qt::Key_Space:
        // マーク/解除
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMarkToggle();
            accepted = true;
        }
        break;

    case Qt::Key_Tab:
        // 隣のパネルに移動
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMoveOther();
            accepted = true;
        }
        break;

    case Qt::Key_Backspace:
        // BS           親フォルダに移動
        // Shift + BS   ルートフォルダに移動
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onMoveParent();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onMoveRoot();
            accepted = true;
        }
        break;

    case Qt::Key_Return:
        // RET          開く
        // Shift + RET  アプリケーションで開く
        if (event->modifiers() == Qt::NoModifier) {
            getMainWnd()->onActionOpen();
            accepted = true;
        }
        else if (event->modifiers() == Qt::ShiftModifier) {
            getMainWnd()->onActionExec();
            accepted = true;
        }
        break;
    }

    if (accepted) {
        event->accept();
    }
    else {
        QTableWidget::keyPressEvent(event);
    }
}

void FileTableWidget::onCellChanged(int row, int column)
{
    if (column == 0) { // マーク状態が変更された
        if (item(row, 0)->checkState() == Qt::Checked) {
            for (int n = 0; n < 4; n++) {
                item(row, n)->setForeground(Qt::red);
                item(row, n)->setBackground(Qt::green);
            }
        }
        else {
            for (int n = 0; n < 4; n++) {
                item(row, n)->setForeground(Qt::black);
                item(row, n)->setBackground(Qt::white);
            }
        }
        selectRow(row);
    }
}
