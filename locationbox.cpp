#include "common.h"
#include "locationbox.h"

#include <QDebug>
#include <QSettings>

LocationBox::LocationBox(QWidget *parent) :
    QLineEdit(parent)
{
}

void LocationBox::initialize()
{
    qDebug() << "LocationBox::initialize";

    updateAppearance();
}

void LocationBox::updateAppearance()
{
    qDebug() << "LocationBox::updateAppearance";

    QSettings settings;

    QPalette pal = palette();
    pal.setColor(QPalette::Base, settings.value(IniKey_BoxColorBg).value<QColor>());
    pal.setColor(QPalette::Text, settings.value(IniKey_BoxColorFg).value<QColor>());
    setPalette(pal);

    setFont(settings.value(IniKey_BoxFont).value<QFont>());
}
