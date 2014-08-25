#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QLineEdit>

class SearchBox : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchBox(QWidget *parent = 0);

signals:
    void searchItem(const QString &text);
    void searchNext(const QString &text);
    void searchPrev(const QString &next);

private slots:
    void itemFound();
    void itemNotFound();

public slots:

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // SEARCHBOX_H
