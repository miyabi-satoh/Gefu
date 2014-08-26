#ifndef SIMPLETEXTVIEW_H
#define SIMPLETEXTVIEW_H

#include <QPlainTextEdit>

class SimpleTextView : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit SimpleTextView(QWidget *parent = 0);

    void setSource(const QByteArray &source);
    void updateAppearance();

private:
    QAction *m_convEUC;
    QAction *m_convJIS;
    QAction *m_convSJIS;
    QAction *m_convUTF8;
    QAction *m_convUTF16;
    QAction *m_convUTF16BE;
    QAction *m_convUTF16LE;
    QAction *m_back;
    QByteArray m_source;

    std::string detectCode(const QByteArray &bytes);

signals:
    void viewFinished(QWidget *sender);

public slots:

private slots:
    void convertFromEUC();
    void convertFromJIS();
    void convertFromSJIS();
    void convertFromUTF8();
    void convertFromUTF16();
    void convertFromUTF16BE();
    void convertFromUTF16LE();
    void back();

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // SIMPLETEXTVIEW_H
