#include "common.h"
#include "simpletextview.h"

#include <QDebug>
#include <QMenu>
#include <QSettings>
#include <QTextCodec>

SimpleTextView::SimpleTextView(QWidget *parent) :
    QPlainTextEdit(parent),
    m_convEUC(NULL),
    m_convJIS(NULL),
    m_convSJIS(NULL),
    m_convUTF8(NULL),
    m_convUTF16BE(NULL),
    m_convUTF16LE(NULL)
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

    connect(m_convSJIS, SIGNAL(triggered()), this, SLOT(convertFromSJIS()));
}

void SimpleTextView::convertFromSJIS()
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    setPlainText(codec->toUnicode(m_source));
}

void SimpleTextView::setSource(const QByteArray &source)
{
    m_source = source;
    convertFromSJIS();
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
    menu.exec(event->globalPos());
}
