#ifndef RENAMESINGLEDIALOG_H
#define RENAMESINGLEDIALOG_H

#include <QDialog>
#include <QDir>

namespace Ui {
class RenameSingleDialog;
}

class RenameSingleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameSingleDialog(QWidget *parent = 0);
    ~RenameSingleDialog();

    void setName(const QString &name);
    void setWorkingDirectory(const QString &dir);

private slots:
    void on_btn_UpperAll_clicked();
    void on_btn_LowerAll_clicked();
    void on_btn_LowerExt_clicked();
    void on_btn_UpperExt_clicked();
    void accept();

private:
    Ui::RenameSingleDialog *ui;
    QDir m_dir;
};

#endif // RENAMESINGLEDIALOG_H
