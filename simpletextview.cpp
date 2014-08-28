#include "common.h"
#include "simpletextview.h"
#include "mainwindow.h"

#include <QDebug>
#include <QMenu>
#include <QSettings>
#include <QShortcut>
#include <QTextCodec>
#include <QStatusBar>

QString KeyEventToSequence(const QKeyEvent *event)
{
    QString modifier = QString::null;
    if (event->modifiers() & Qt::ShiftModifier)     { modifier += "Shift+"; }
    if (event->modifiers() & Qt::ControlModifier)   { modifier += "Ctrl+"; }
    if (event->modifiers() & Qt::AltModifier)       { modifier += "Alt+"; }
    if (event->modifiers() & Qt::MetaModifier)      { modifier += "Meta+"; }

    QString key = QKeySequence(event->key()).toString();
    return QKeySequence(modifier + key).toString();
}

SimpleTextView::SimpleTextView(QWidget *parent) :
    QPlainTextEdit(parent),
    m_convEUC(NULL),
    m_convJIS(NULL),
    m_convSJIS(NULL),
    m_convUTF8(NULL),
    m_convUTF16BE(NULL),
    m_convUTF16LE(NULL),
    m_copy(NULL),
    m_back(NULL)
{
    setReadOnly(true);
    updateAppearance();

    setContextMenuPolicy(Qt::DefaultContextMenu);
    m_convEUC = new QAction(tr("EUC-JPで再読込"), this);
    m_convJIS = new QAction(tr("ISO 2022-JP(JIS)で再読込"), this);
    m_convSJIS = new QAction(tr("Shift-JISで再読込"), this);
    m_convUTF8 = new QAction(tr("UTF-8で再読込"), this);
    m_convUTF16 = new QAction(tr("UTF-16で再読込"), this);
    m_convUTF16BE = new QAction(tr("UTF-16BEで再読込"), this);
    m_convUTF16LE = new QAction(tr("UTF-16LEで再読込"), this);
    m_copy = new QAction(tr("選択範囲をクリップボードにコピー"), this);
    m_back = new QAction(tr("戻る"), this);

    m_convEUC->setObjectName("convertFromEUC");
    m_convJIS->setObjectName("convertFromJIS");
    m_convSJIS->setObjectName("convertFromSJIS");
    m_convUTF8->setObjectName("convertFromUTF8");
    m_convUTF16->setObjectName("convertFromUTF16");
    m_convUTF16BE->setObjectName("convertFromUTF16BE");
    m_convUTF16LE->setObjectName("convertFromUTF16LE");
    m_copy->setObjectName("copy");
    m_back->setObjectName("back");

    m_convEUC->setShortcut(QKeySequence("Shift+E"));
    m_convJIS->setShortcut(QKeySequence("Shift+J"));
    m_convSJIS->setShortcut(QKeySequence("Shift+S"));
    m_convUTF8->setShortcut(QKeySequence("Shift+U"));
    m_convUTF16->setShortcut(QKeySequence("Shift+I"));
    m_convUTF16BE->setShortcut(QKeySequence("Shift+O"));
    m_convUTF16LE->setShortcut(QKeySequence("Shift+P"));
    m_copy->setShortcut(QKeySequence::Copy);

    QList<QKeySequence> shortcuts;
    shortcuts << QKeySequence("Return");
    shortcuts << QKeySequence("Backspace");
    m_back->setShortcuts(shortcuts);

    connect(m_convEUC, SIGNAL(triggered()), this, SLOT(convertFromEUC()));
    connect(m_convJIS, SIGNAL(triggered()), this, SLOT(convertFromJIS()));
    connect(m_convSJIS, SIGNAL(triggered()), this, SLOT(convertFromSJIS()));
    connect(m_convUTF8, SIGNAL(triggered()), this, SLOT(convertFromUTF8()));
    connect(m_convUTF16, SIGNAL(triggered()), this, SLOT(convertFromUTF16()));
    connect(m_convUTF16BE, SIGNAL(triggered()), this, SLOT(convertFromUTF16BE()));
    connect(m_convUTF16LE, SIGNAL(triggered()), this, SLOT(convertFromUTF16LE()));
    connect(m_copy, SIGNAL(triggered()), this, SLOT(copy()));
    connect(m_back, SIGNAL(triggered()), this, SLOT(back()));

    connect(this, SIGNAL(copyAvailable(bool)), this, SLOT(onCopyAvailable(bool)));

    m_copy->setEnabled(false);
}

void SimpleTextView::setSource(const QByteArray &source)
{
    m_source = source;

    std::string code = detectCode(m_source.left(1024));
    QTextCodec *codec = QTextCodec::codecForName(code.c_str());

    setPlainText(codec->toUnicode(m_source));

    emit fileInfo(code.c_str());
}

void SimpleTextView::updateAppearance()
{
    QSettings settings;

    QPalette pal = this->palette();
    if (settings.value(IniKey_ViewerInherit).toBool()) {
        pal.setColor(QPalette::Base,
                     settings.value(IniKey_ViewColorBgNormal).value<QColor>());
        pal.setColor(QPalette::Text,
                     settings.value(IniKey_ViewColorFgNormal).value<QColor>());
    }
    else {
        pal.setColor(QPalette::Base,
                     settings.value(IniKey_ViewerColorBg).value<QColor>());
        pal.setColor(QPalette::Text,
                     settings.value(IniKey_ViewerColorFg).value<QColor>());
    }
    setPalette(pal);
    setFont(settings.value(IniKey_ViewerFont).value<QFont>());
}

void SimpleTextView::convertFromEUC()
{
    QTextCodec *codec = QTextCodec::codecForName("EUC-JP");
    setPlainText(codec->toUnicode(m_source));
    emit fileInfo("EUC-JP");
}

void SimpleTextView::convertFromJIS()
{
    QTextCodec *codec = QTextCodec::codecForName("ISO 2022-JP");
    setPlainText(codec->toUnicode(m_source));
    emit fileInfo("ISO 2022-JP");
}

void SimpleTextView::convertFromSJIS()
{
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    setPlainText(codec->toUnicode(m_source));
    emit fileInfo("Shift-JIS");
}

void SimpleTextView::convertFromUTF8()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    setPlainText(codec->toUnicode(m_source));
    emit fileInfo("UTF-8");
}

void SimpleTextView::convertFromUTF16()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-16");
    setPlainText(codec->toUnicode(m_source));
    emit fileInfo("UTF-16");
}

void SimpleTextView::convertFromUTF16BE()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-16BE");
    setPlainText(codec->toUnicode(m_source));
    emit fileInfo("UTF-16BE");
}

void SimpleTextView::convertFromUTF16LE()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-16LE");
    setPlainText(codec->toUnicode(m_source));
    emit fileInfo("UTF-16LE");
}

void SimpleTextView::onCopyAvailable(bool yes)
{
    m_copy->setEnabled(yes);
}

void SimpleTextView::back()
{
    emit viewFinished();
}

void SimpleTextView::keyPressEvent(QKeyEvent *event)
{
    QString ksq = KeyEventToSequence(event);

    if (ProcessShortcut(ksq, this)) {
        event->accept();
        return;
    }

    QPlainTextEdit::keyPressEvent(event);

    // MainWindowへ
    event->ignore();
}


void SimpleTextView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << "SimpleTextView::contextMenuEvent();";

    QMenu menu(this);
    menu.addAction(m_convEUC);
    menu.addAction(m_convJIS);
    menu.addAction(m_convSJIS);
    menu.addAction(m_convUTF8);
    menu.addAction(m_convUTF16BE);
    menu.addAction(m_convUTF16LE);
    menu.addSeparator();
    menu.addAction(m_copy);
    menu.addSeparator();
    menu.addAction(m_back);
    menu.exec(event->globalPos());
}

// http://dobon.net/vb/dotnet/string/detectcode.html より拝借
std::string SimpleTextView::detectCode(const QByteArray &bytes)
{
    typedef unsigned char byte;
    const byte bEscape = 0x1B;
    const byte bAt = 0x40;
    const byte bDollar = 0x24;
    const byte bAnd = 0x26;
    const byte bOpen = 0x28;    //'('
    const byte bB = 0x42;
    const byte bD = 0x44;
    const byte bJ = 0x4A;
    const byte bI = 0x49;

    int len = bytes.size();
    byte b1, b2, b3, b4;

    bool isBinary = false;
    for (int i = 0; i < len; i++) {
        b1 = bytes[i];
        if (b1 <= 0x06 || b1 == 0x7F || b1 == 0xFF) {
            //'binary'
            isBinary = true;
            if (b1 == 0x00 && i < len - 1 && static_cast<byte>(bytes[i + 1]) <= 0x7F) {
                return "UTF-16LE";
            }
        }
    }
    if (isBinary) {
        return "UTF-8";
    }

    bool notJapanese = true;
    for (int i = 0; i < len; i++) {
        b1 = bytes[i];
        if (b1 == bEscape || 0x80 <= b1) {
            notJapanese = false;
            break;
        }
    }
    if (notJapanese) {
        return "UTF-8";
    }

    for (int i = 0; i < len - 2; i++) {
        b1 = bytes[i];
        b2 = bytes[i + 1];
        b3 = bytes[i + 2];

        if (b1 == bEscape){
            if ((b2 == bDollar && b3 == bAt) ||
                (b2 == bDollar && b3 == bB) ||
                (b2 == bOpen && (b3 == bB || b3 == bJ)) ||
                (b2 == bOpen && b3 == bI))
            {
                return "ISO 2022-JP";
            }
            if (i < len - 3) {
                b4 = bytes[i + 3];
                if (b2 == bDollar && b3 == bOpen && b4 == bD) {
                    return "ISO 2022-JP";
                }
                if (i < len - 5 &&
                    b2 == bAnd && b3 == bAt && b4 == bEscape &&
                    bytes[i + 4] == bDollar && bytes[i + 5] == bB)
                {
                    return "ISO 2022-JP";
                }
            }
        }
    }

    int sjis = 0;
    int euc = 0;
    int utf8 = 0;
    for (int i = 0; i < len - 1; i++) {
        b1 = bytes[i];
        b2 = bytes[i + 1];
        if (((0x81 <= b1 && b1 <= 0x9F) || (0xE0 <= b1 && b1 <= 0xFC)) &&
            ((0x40 <= b2 && b2 <= 0x7E) || (0x80 <= b2 && b2 <= 0xFC)))
        {
            sjis += 2;
            i++;
        }
    }
    for (int i = 0; i < len - 1; i++) {
        b1 = bytes[i];
        b2 = bytes[i + 1];
        if (((0xA1 <= b1 && b1 <= 0xFE) && (0xA1 <= b2 && b2 <= 0xFE)) ||
            (b1 == 0x8E && (0xA1 <= b2 && b2 <= 0xDF)))
        {
            euc += 2;
            i++;
        }
        else if (i < len - 2) {
            b3 = bytes[i + 2];
            if (b1 == 0x8F && (0xA1 <= b2 && b2 <= 0xFE) &&
                (0xA1 <= b3 && b3 <= 0xFE))
            {
                euc += 3;
                i += 2;
            }
        }
    }
    for (int i = 0; i < len - 1; i++) {
        b1 = bytes[i];
        b2 = bytes[i + 1];
        if ((0xC0 <= b1 && b1 <= 0xDF) && (0x80 <= b2 && b2 <= 0xBF)) {
            utf8 += 2;
            i++;
        }
        else if (i < len - 2) {
            b3 = bytes[i + 2];
            if ((0xE0 <= b1 && b1 <= 0xEF) && (0x80 <= b2 && b2 <= 0xBF) &&
                (0x80 <= b3 && b3 <= 0xBF))
            {
                utf8 += 3;
                i += 2;
            }
        }
    }

    if (euc > sjis && euc > utf8) {
        return "EUC-JP";
    }
    else if (sjis > euc && sjis > utf8) {
        return "Shift-JIS";
    }
    else if (utf8 > euc && utf8 > sjis) {
        return "UTF-8";
    }

#ifdef Q_OS_WIN
    return "Shift-JIS";
#else
    return "UTF-8";
#endif
}
