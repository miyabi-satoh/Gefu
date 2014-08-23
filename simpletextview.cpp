#include "common.h"
#include "simpletextview.h"

#include <QDebug>
#include <QSettings>

SimpleTextView::SimpleTextView(QWidget *parent) :
    QPlainTextEdit(parent)
{
    setReadOnly(true);

    QSettings settings;
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base,
                     settings.value(IniKey_ViewColorBgNormal).value<QColor>());
    palette.setColor(QPalette::Text,
                     settings.value(IniKey_ViewColorFgNormal).value<QColor>());
    setPalette(palette);
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
