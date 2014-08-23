#ifndef SIMPLETEXTVIEW_H
#define SIMPLETEXTVIEW_H

#include <QPlainTextEdit>

class SimpleTextView : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit SimpleTextView(QWidget *parent = 0);

    void setSource(const QByteArray &source);

private:
    QAction *m_convEUC;
    QAction *m_convJIS;
    QAction *m_convSJIS;
    QAction *m_convUTF8;
    QAction *m_convUTF16BE;
    QAction *m_convUTF16LE;
    QByteArray m_source;

signals:
    void viewFinished(QWidget *sender);

public slots:

private slots:
    void convertFromEUC();
    void convertFromJIS();
    void convertFromSJIS();
    void convertFromUTF8();
    void convertFromUTF16BE();
    void convertFromUTF16LE();

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // SIMPLETEXTVIEW_H
