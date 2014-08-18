#ifndef SORTDIALOG_H
#define SORTDIALOG_H

#include <QDialog>

namespace Ui {
class SortDialog;
}

class SortDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SortDialog(QWidget *parent = 0);
    ~SortDialog();

    void setRightOrLeft(const QString &s);

protected:
    void accept();

private:
    Ui::SortDialog *ui;
    QString m_RightOrLeft;
};

#endif // SORTDIALOG_H
