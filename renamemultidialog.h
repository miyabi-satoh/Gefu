#ifndef RENAMEMULTIDIALOG_H
#define RENAMEMULTIDIALOG_H

#include <QAbstractButton>
#include <QDialog>
#include <QDir>

namespace Ui {
class RenameMultiDialog;
}

class RenameMultiDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenameMultiDialog(QWidget *parent = 0);
    ~RenameMultiDialog();

    void setNames(const QStringList &names);
    void setWorkingDirectory(const QString &dir);

private slots:
    void on_btn_UpperAll_clicked();
    void on_btn_LowerAll_clicked();
    void on_btn_UpperExt_clicked();
    void on_btn_LowerExt_clicked();
    void on_replaceExec_clicked();
    void on_numberExec_clicked();
    void on_buttonBox_clicked(QAbstractButton *button);
    void accept();

private:
    Ui::RenameMultiDialog *ui;
    QDir m_dir;
};

#endif // RENAMEMULTIDIALOG_H
