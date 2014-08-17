#ifndef RENAMEMULTIDIALOG_H
#define RENAMEMULTIDIALOG_H

#include <QAbstractButton>
#include "irenamedialog.h"

namespace Ui {
class RenameMultiDialog;
}

class RenameMultiDialog : public IRenameDialog
{
    Q_OBJECT

public:
    explicit RenameMultiDialog(QWidget *parent = 0);
    ~RenameMultiDialog();

    void setNames(const QStringList &names);

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
};

#endif // RENAMEMULTIDIALOG_H
