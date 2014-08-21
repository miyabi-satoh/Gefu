#include "clickablelabel.h"

ClickableLabel::ClickableLabel(QWidget *parent) :
    QLabel(parent)
{
}


void ClickableLabel::mouseReleaseEvent(QMouseEvent *)
{
    emit clicked();
}
