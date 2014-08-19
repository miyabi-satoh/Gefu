#include "common.h"
#include "mainwindow.h"
#include "filetablewidget.h"
#include <QKeyEvent>
#include <QHeaderView>
#include <folderpanel.h>
#include <QAction>
#include <QDebug>
#include <QMenuBar>

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
    // Macでアクションが処理されないケースがあるため、
    // キーイベントを拾ってアクションシグナルを起動する
    QString modifier = QString::null;
    if (event->modifiers() & Qt::ShiftModifier)     { modifier += "Shift+"; }
    if (event->modifiers() & Qt::ControlModifier)   { modifier += "Ctrl+"; }
    if (event->modifiers() & Qt::AltModifier)       { modifier += "Alt+"; }
    if (event->modifiers() & Qt::MetaModifier)      { modifier += "Meta+"; }

    QString key = QKeySequence(event->key()).toString();
    QString ksq = QKeySequence(modifier + key).toString();

    if (!ksq.isEmpty()) {
        foreach (QObject *obj, getMainWnd()->children()) {
            QAction *action = qobject_cast<QAction*>(obj);
            if (action) {
                if (ksq == action->shortcut().toString()) {
                    qDebug() << "emit " << ksq << " " << action->objectName();
                    emit action->triggered();
                    event->accept();
                    return;
                }
            }
        }
    }

    qDebug() << ksq;
    QTableWidget::keyPressEvent(event);
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

const QString& FileTableWidget::side() const
{
    FolderPanel *fp = static_cast<FolderPanel*>(parentWidget());
    return fp->side();
}
