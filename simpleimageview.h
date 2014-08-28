#ifndef SIMPLEIMAGEVIEW_H
#define SIMPLEIMAGEVIEW_H

#include <QGraphicsView>

class SimpleImageView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SimpleImageView(QWidget *parent = 0);

    bool setSource(const QString &path);

private:
    QPixmap m_img;
    QAction *m_back;

signals:
    void viewFinished();
    void fileInfo(const QString &info);

public slots:
    void back();

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *);
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // SIMPLEIMAGEVIEW_H
