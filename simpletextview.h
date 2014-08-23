#ifndef SIMPLETEXTVIEW_H
#define SIMPLETEXTVIEW_H

#include <QPlainTextEdit>

class SimpleTextView : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit SimpleTextView(QWidget *parent = 0);

signals:
    void viewFinished(QWidget *sender);

public slots:


    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // SIMPLETEXTVIEW_H
