#ifndef SIMPLEIMAGEVIEW_H
#define SIMPLEIMAGEVIEW_H

#include <QGraphicsView>

class SimpleImageView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit SimpleImageView(QWidget *parent = 0);

    bool setSource(const QString &path);

    void changeScale(bool up);

private:
    QAction *m_back;
    QAction *m_fitToWindow;
    QAction *m_scaleUp;
    QAction *m_scaleDown;
    QAction *m_scaleNormal;
    QAction *m_rot90;
    QAction *m_rot180;
    QPixmap m_imgSrc;
    double m_scaleFactor;
    int m_rotateDeg;

private:
    double scaleFactor(const QSize &size);
    void updateActions();
    void sizeChanged();
    double resizeImage();

signals:
    void viewFinished();
    void fileInfo(const QString &info);

private slots:
    void fitToWindow(bool checked);
    void scaleNormal();
    void scaleUp();
    void scaleDown();
    void rotate90();
    void rotate180();

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // SIMPLEIMAGEVIEW_H
