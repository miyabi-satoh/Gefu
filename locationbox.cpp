#include "common.h"
#include "locationbox.h"

#include <QDebug>
#include <QSettings>

LocationBox::LocationBox(QWidget *parent) :
    QLineEdit(parent)
{
}

void LocationBox::initialize(bool left)
{
    qDebug() << "LocationBox::initialize";

    updateAppearance(!left);
}

void LocationBox::updateAppearance(bool darker)
{
    qDebug() << "LocationBox::updateAppearance";

    QSettings settings;

    QPalette pal = palette();
    int darkness = 100;
    if (darker && settings.value(IniKey_EnableDarker).toBool()) {
        darkness += settings.value(IniKey_Darkness).toInt();
    }
    pal.setColor(QPalette::Base, settings.value(IniKey_BoxColorBg).value<QColor>().darker(darkness));
    pal.setColor(QPalette::Text, settings.value(IniKey_BoxColorFg).value<QColor>().darker(darkness));
    setPalette(pal);

    setFont(settings.value(IniKey_BoxFont).value<QFont>());
}
