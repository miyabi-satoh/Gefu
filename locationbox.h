#ifndef LOCATIONBOX_H
#define LOCATIONBOX_H

#include <QLineEdit>

class LocationBox : public QLineEdit
{
    Q_OBJECT
public:
    explicit LocationBox(QWidget *parent = 0);

    void initialize();
    void updateAppearance();

signals:

public slots:

};

#endif // LOCATIONBOX_H
