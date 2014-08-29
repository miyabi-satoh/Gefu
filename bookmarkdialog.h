#ifndef BOOKMARKDIALOG_H
#define BOOKMARKDIALOG_H

#include <QDialog>

namespace Ui {
class BookmarkDialog;
}

class BookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarkDialog(QWidget *parent = 0);
    ~BookmarkDialog();

    void setEditMode(bool edit);
    int selectedIndex() const;

private:
    Ui::BookmarkDialog *ui;
    bool m_isReadOnly;

    void insertData(int row, const QString &name, const QString &path);
    void moveSelectedRows(bool up);

    // QDialog interface
public slots:
    void accept();

private slots:
    void on_buttonDelete_clicked();
    void on_buttonUp_clicked();
    void on_buttonDown_clicked();
    void on_buttonAdd_clicked();
};

#endif // BOOKMARKDIALOG_H
