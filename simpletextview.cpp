#include "common.h"
#include "simpletextview.h"
#include "mainwindow.h"

#include <QDebug>
#include <QMenu>
#include <QSettings>
#include <QShortcut>
#include <QTextCodec>

SimpleTextView::SimpleTextView(QWidget *parent) :
    QPlainTextEdit(parent),
    m_convEUC(NULL),
    m_convJIS(NULL),
    m_convSJIS(NULL),
    m_convUTF8(NULL),
    m_convUTF16BE(NULL),
    m_convUTF16LE(NULL),
    m_back(NULL)
{
    setReadOnly(true);

    QSettings settings;
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base,
                     settings.value(IniKey_ViewColorBgNormal).value<QColor>());
    palette.setColor(QPalette::Text,
                     settings.value(IniKey_ViewColorFgNormal).value<QColor>());
    setPalette(palette);

    QFont font = this->font();
#ifdef Q_OS_MAC
    font.setFamily("Menlo");
#else
    font.setFamily("ＭＳ ゴシック");
#endif
    setFont(font);

    setContextMenuPolicy(Qt::DefaultContextMenu);
    m_convEUC = new QAction(tr("EUC-JPで再読込"), this);
    m_convJIS = new QAction(tr("ISO 2022-JP(JIS)で再読込"), this);
    m_convSJIS = new QAction(tr("Shift-JISで再読込"), this);
    m_convUTF8 = new QAction(tr("UTF-8で再読込"), this);
    m_convUTF16BE = new QAction(tr("UTF-16BEで再読込"), this);
    m_convUTF16LE = new QAction(tr("UTF-16LEで再読込"), this);
    m_back = new QAction(tr("戻る"), this);

    m_convEUC->setShortcut(QKeySequence("E"));
    m_convJIS->setShortcut(QKeySequence("J"));
    m_convSJIS->setShortcut(QKeySequence("S"));
    m_convUTF8->setShortcut(QKeySequence("U"));
    m_convUTF16BE->setShortcut(QKeySequence("B"));
    m_convUTF16LE->setShortcut(QKeySequence("L"));
    m_back->setShortcut(QKeySequence("Return"));

    connect(m_convEUC, SIGNAL(triggered()), this, SLOT(convertFromEUC()));
    connect(m_convJIS, SIGNAL(triggered()), this, SLOT(convertFromJIS()));
    connect(m_convSJIS, SIGNAL(triggered()), this, SLOT(convertFromSJIS()));
    connect(m_convUTF8, SIGNAL(triggered()), this, SLOT(convertFromUTF8()));
    connect(m_convUTF16BE, SIGNAL(triggered()), this, SLOT(convertFromUTF16BE()));
    connect(m_convUTF16LE, SIGNAL(triggered()), this, SLOT(convertFromUTF16LE()));
    connect(m_back, SIGNAL(triggered()), this, SLOT(back()));
}

void SimpleTextView::setSource(const QByteArray &source)
{
    m_source = source;
#ifdef Q_OS_MAC
    convertFromUTF8();
#else
    convertFromSJIS();
#endif
}

void SimpleTextView::convertFromEUC()
{
    QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
    setPlainText(codec->toUnicode(m_source));
}

void SimpleTextView::convertFromJIS()
{
    QTextCodec *codec = QTextCodec::codecForName("ISO 2022-JP");
    setPlainText(codec->toUnicode(m_source));
}

void SimpleTextView::convertFromSJIS()
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    setPlainText(codec->toUnicode(m_source));
}

void SimpleTextView::convertFromUTF8()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    setPlainText(codec->toUnicode(m_source));
}

void SimpleTextView::convertFromUTF16BE()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-16BE");
    setPlainText(codec->toUnicode(m_source));
}

void SimpleTextView::convertFromUTF16LE()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    setPlainText(codec->toUnicode(m_source));
}

void SimpleTextView::back()
{
    emit viewFinished(this);
}

void SimpleTextView::keyPressEvent(QKeyEvent *event)
{
    QString modifier = QString::null;
    if (event->modifiers() & Qt::ShiftModifier)     { modifier += "Shift+"; }
    if (event->modifiers() & Qt::ControlModifier)   { modifier += "Ctrl+"; }
    if (event->modifiers() & Qt::AltModifier)       { modifier += "Alt+"; }
    if (event->modifiers() & Qt::MetaModifier)      { modifier += "Meta+"; }

    QString key = QKeySequence(event->key()).toString();
    QString ksq = QKeySequence(modifier + key).toString();

    if (ksq == "Return" || ksq == "Backspace" || ksq == "W") {
        emit viewFinished(this);
        event->accept();
        return;
    }

    if (!ksq.isEmpty()) {
        foreach (QObject *obj, this->children()) {
            QAction *action = qobject_cast<QAction*>(obj);
            if (action && action->isEnabled()) {
                foreach (const QKeySequence &keySeq, action->shortcuts()) {
                    if (ksq == keySeq.toString()) {
                        qDebug() << "emit " << ksq << " " << action->objectName();
                        emit action->triggered();
                        event->accept();
                        return;
                    }
                }
            }
        }

        foreach (QObject *obj, getMainWnd()->children()) {
            QAction *action = qobject_cast<QAction*>(obj);
            if (action && action->isEnabled()) {
                foreach (const QKeySequence &keySeq, action->shortcuts()) {
                    if (ksq == keySeq.toString()) {
                        qDebug() << "emit " << ksq << " " << action->objectName();
                        emit action->triggered();
                        event->accept();
                        return;
                    }
                }
            }
        }
    }

    if (ksq != "Down" && ksq != "Up") {
        qDebug() << ksq;
    }

    QPlainTextEdit::keyPressEvent(event);
}


void SimpleTextView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "contextMenuEvent();";
    QMenu menu(this);
    menu.addAction(m_convEUC);
    menu.addAction(m_convJIS);
    menu.addAction(m_convSJIS);
    menu.addAction(m_convUTF8);
    menu.addAction(m_convUTF16BE);
    menu.addAction(m_convUTF16LE);
    menu.addAction(m_back);
    menu.exec(event->globalPos());
}
