#include "common.h"
#include "mainwindow.h"
#include "searchbox.h"

#include <QAction>
#include <QDebug>
#include <QKeyEvent>
#include <QApplication>

SearchBox::SearchBox(QWidget *parent) :
    QLineEdit(parent)
{
}

void SearchBox::itemFound()
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Text, Qt::black);
    setPalette(palette);
}

void SearchBox::itemNotFound()
{
    QPalette palette = this->palette();
    palette.setColor(QPalette::Text, Qt::red);
    setPalette(palette);
}

void SearchBox::keyPressEvent(QKeyEvent *event)
{
#if 0
    QString ksq = KeyEventToSequence(event);

    if (ksq == "/") {
        QAction *action = getMainWnd()->findChild<QAction*>("action_Search");
        action->toggle();
        event->accept();
        return;
    }

    QString textBefore = text();
    QLineEdit::keyPressEvent(event);


    if (ksq == "Shift+Return"){
        qDebug() << ksq;
        emit getMainWnd()->findChild<QAction*>("action_SearchPrev")->triggered();
    }
    else if (ksq.indexOf("Return") != -1) {
        qDebug() << ksq;
        emit getMainWnd()->findChild<QAction*>("action_SearchNext")->triggered();
    }
    else if (textBefore != text()) {
        emit searchItem(text());
    }
    else {
        qDebug() << ksq;
    }
#endif

    QLineEdit::keyPressEvent(event);

}
