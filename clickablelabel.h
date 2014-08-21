#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickableLabel(QWidget *parent = 0);

signals:
    void clicked();

public slots:


    // QWidget interface
protected:
    void mouseReleaseEvent(QMouseEvent *);
};

#endif // CLICKABLELABEL_H
