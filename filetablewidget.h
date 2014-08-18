#ifndef FILETABLEWIDGET_H
#define FILETABLEWIDGET_H

#include <QTableWidget>

class FileTableWidget : public QTableWidget
{
    Q_OBJECT
public:
    explicit FileTableWidget(QWidget *parent = 0);

signals:

public slots:
    void onCellChanged(int row, int column);

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // FILETABLEWIDGET_H
