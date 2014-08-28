#include "common.h"
#include "simpleimageview.h"

#include <QDebug>
#include <QKeyEvent>
#include <QAction>
#include <QMenu>

SimpleImageView::SimpleImageView(QWidget *parent) :
    QGraphicsView(parent),
    m_img(),
    m_back(NULL)
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
    m_back = new QAction(tr("戻る"), this);
    m_back->setObjectName("back");

    QList<QKeySequence> shortcuts;
    shortcuts << QKeySequence("Return");
    shortcuts << QKeySequence("Backspace");
    m_back->setShortcuts(shortcuts);

    connect(m_back, SIGNAL(triggered()), this, SLOT(back()));
}

bool SimpleImageView::setSource(const QString &path)
{
    qDebug() << "SimpleImageView::setSource()";

    m_img = QImage(path);

    emit fileInfo(QString("%1x%2x%3bpp")
                  .arg(m_img.width())
                  .arg(m_img.height())
                  .arg(m_img.depth()));

    return !m_img.isNull();
}

void SimpleImageView::back()
{
    emit viewFinished();
}

void SimpleImageView::paintEvent(QPaintEvent *)
{
    qDebug() << "SimpleImageView::paintEvent();";

    QPainter painter(viewport());
    QImage scaledImg;

    if (m_img.width() < viewport()->width() &&
        m_img.height() < viewport()->height())
    {
        scaledImg = m_img;
    }
    else {
        scaledImg = m_img.scaled(viewport()->size(),
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
    }

    painter.drawImage(
                (viewport()->width() - scaledImg.width()) / 2,
                (viewport()->height() - scaledImg.height()) / 2,
                scaledImg);
}

void SimpleImageView::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "SimpleImageView::keyPressEvent();";

    QString ksq = KeyEventToSequence(event);

    if (ProcessShortcut(ksq, this)) {
        event->accept();
        return;
    }

    QGraphicsView::keyReleaseEvent(event);

    // MainWindowへ
    event->ignore();
}

void SimpleImageView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "SimpleImageView::contextMenuEvent();";

    QMenu menu(this);
    menu.addAction(m_back);
    menu.exec(event->globalPos());
}
