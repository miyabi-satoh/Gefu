#include "common.h"
#include "simpleimageview.h"

#include <QDebug>
#include <QSettings>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QResizeEvent>
#include <QAction>
#include <QMenu>

SimpleImageView::SimpleImageView(QWidget *parent) :
    QGraphicsView(parent),
    m_back(NULL),
    m_fitToWindow(NULL),
    m_scaleUp(NULL),
    m_scaleDown(NULL),
    m_scaleNormal(NULL),
    m_rot90(NULL),
    m_rot180(NULL),
    m_imgSrc(),
    m_scaleFactor(0),
    m_rotateDeg(0)
{
    QSettings settings;
    QPalette pal = palette();
    pal.setColor(QPalette::Base, settings.value(IniKey_ViewColorBgNormal).value<QColor>());
    setBackgroundRole(QPalette::Base);
    setPalette(pal);

    setContextMenuPolicy(Qt::DefaultContextMenu);

    QList<QKeySequence> shortcuts;
    //>>>>> 「戻る」メニュー
    m_back = new QAction(tr("戻る"), this);
    m_back->setObjectName("back");
    shortcuts.clear();
    shortcuts << QKeySequence("Return");
    shortcuts << QKeySequence("Backspace");
    m_back->setShortcuts(shortcuts);

    //>>>>> 「ウィンドウにフィット」メニュー
    m_fitToWindow = new QAction(tr("ウィンドウにフィット"), this);
    m_fitToWindow->setObjectName("fitToWindow");
    m_fitToWindow->setCheckable(true);
    m_fitToWindow->setChecked(true);
    m_fitToWindow->setShortcut(QKeySequence("Space"));

    //>>>>> 「拡大」メニュー
    m_scaleUp = new QAction(tr("拡大"), this);
    m_scaleUp->setObjectName("scaleUp");
    shortcuts.clear();
    shortcuts << QKeySequence("+");
    shortcuts << QKeySequence("Shift++");
    m_scaleUp->setShortcuts(shortcuts);

    //>>>>> 「縮小」メニュー
    m_scaleDown = new QAction(tr("縮小"), this);
    m_scaleDown->setObjectName("scaleDown");
    m_scaleDown->setShortcut(QKeySequence("-"));

    //>>>>> 「等倍」メニュー
    m_scaleNormal = new QAction(tr("等倍"), this);
    m_scaleNormal->setObjectName("scaleUp");
    shortcuts.clear();
    shortcuts << QKeySequence("=");
    shortcuts << QKeySequence("Shift+=");
    m_scaleNormal->setShortcuts(shortcuts);

    //>>>>> 「右に90度回転」メニュー
    m_rot90 = new QAction(tr("右に90度回転"), this);
    m_rot90->setObjectName("rot90");
    m_rot90->setShortcut(QKeySequence("9"));

    //>>>>> 「右に180度回転」メニュー
    m_rot180 = new QAction(tr("右に180度回転"), this);
    m_rot180->setObjectName("rot180");
    m_rot180->setShortcut(QKeySequence("0"));

    connect(m_back, SIGNAL(triggered()), this, SIGNAL(viewFinished()));
    connect(m_fitToWindow, SIGNAL(toggled(bool)), this, SLOT(fitToWindow(bool)));
    connect(m_scaleDown, SIGNAL(triggered()), this, SLOT(scaleDown()));
    connect(m_scaleNormal, SIGNAL(triggered()), this, SLOT(scaleNormal()));
    connect(m_scaleUp, SIGNAL(triggered()), this, SLOT(scaleUp()));
    connect(m_rot90, SIGNAL(triggered()), this, SLOT(rotate90()));
    connect(m_rot180, SIGNAL(triggered()), this, SLOT(rotate180()));

    setDragMode(ScrollHandDrag);
}

bool SimpleImageView::setSource(const QString &path)
{
    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        return false;
    }

    m_imgSrc = pixmap;
    m_rotateDeg = 0;
    m_scaleFactor = 0;
    m_fitToWindow->blockSignals(true);
    m_fitToWindow->setChecked(true);
    m_fitToWindow->blockSignals(false);

    QGraphicsScene *scene = new QGraphicsScene(this);
    scene->addPixmap(pixmap);
    setScene(scene);
    return true;
}

double SimpleImageView::scaleFactor(const QSize &size)
{
    double scaleFactor;
    if (m_scaleFactor == 0) {
        double scaleX, scaleY;
        if (m_rotateDeg == 90 || m_rotateDeg == 270) {
            scaleX = 1.0 * size.width() / m_imgSrc.height();
            scaleY = 1.0 * size.height() / m_imgSrc.width();
        }
        else {
            scaleX = 1.0 * size.width() / m_imgSrc.width();
            scaleY = 1.0 * size.height() / m_imgSrc.height();
        }
        scaleFactor =  (scaleX > scaleY) ? scaleY : scaleX;
    }
    else {
        scaleFactor = m_scaleFactor;
    }

    return scaleFactor;
}

void SimpleImageView::sizeChanged()
{
    QString str;
    str = tr("%1px x %2px x %3bpp(%4%)")
            .arg(m_imgSrc.width())
            .arg(m_imgSrc.height())
            .arg(m_imgSrc.depth())
            .arg(int(scaleFactor(viewport()->size()) * 100));
    emit fileInfo(str);
}

double SimpleImageView::resizeImage()
{
    double scaleFactor = this->scaleFactor(viewport()->size());
    m_rotateDeg %= 360;

    this->setTransform(QTransform());
    this->translate(width() / 2, height() / 2);
    this->rotate(m_rotateDeg);
    this->scale(scaleFactor, scaleFactor);
    sizeChanged();

    return scaleFactor;
}

void SimpleImageView::fitToWindow(bool checked)
{
    if (checked) {
        m_scaleFactor = 0;
    }

    double scaleFactor = resizeImage();

    if (!checked) {
        m_scaleFactor = scaleFactor;
    }

    sizeChanged();
}

void SimpleImageView::scaleNormal()
{
    m_scaleFactor = 1;
    resizeImage();

    m_fitToWindow->blockSignals(true);
    m_fitToWindow->setChecked(false);
    m_fitToWindow->blockSignals(false);
}

void SimpleImageView::scaleUp()
{
    m_scaleFactor = scaleFactor(viewport()->size()) * 1.25;
    resizeImage();

    m_fitToWindow->blockSignals(true);
    m_fitToWindow->setChecked(false);
    m_fitToWindow->blockSignals(false);
}

void SimpleImageView::scaleDown()
{
    m_scaleFactor = scaleFactor(viewport()->size()) * 0.8;
    resizeImage();

    m_fitToWindow->blockSignals(true);
    m_fitToWindow->setChecked(false);
    m_fitToWindow->blockSignals(false);
}

void SimpleImageView::rotate90()
{
    m_rotateDeg += 90;
    resizeImage();
}

void SimpleImageView::rotate180()
{
    m_rotateDeg += 180;
    resizeImage();
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
    menu.addAction(m_fitToWindow);
    menu.addAction(m_scaleUp);
    menu.addAction(m_scaleDown);
    menu.addAction(m_scaleNormal);
    menu.addSeparator();
    menu.addAction(m_rot90);
    menu.addAction(m_rot180);
    menu.addSeparator();
    menu.addAction(m_back);
    menu.exec(event->globalPos());
}

void SimpleImageView::resizeEvent(QResizeEvent *event)
{
    if (!m_imgSrc.isNull()) {
        resizeImage();
    }
    QGraphicsView::resizeEvent(event);
}
