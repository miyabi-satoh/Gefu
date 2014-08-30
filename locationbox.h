#ifndef LOCATIONBOX_H
#define LOCATIONBOX_H

#include <QLineEdit>

class LocationBox : public QLineEdit
{
    Q_OBJECT
public:
    explicit LocationBox(QWidget *parent = 0);

    void initialize(bool left);
    void updateAppearance(bool darker = false);

signals:

public slots:

};

#endif // LOCATIONBOX_H
