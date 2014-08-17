#ifndef RENAMESINGLEDIALOG_H
#define RENAMESINGLEDIALOG_H

#include "irenamedialog.h"

namespace Ui {
class RenameSingleDialog;
}

class RenameSingleDialog : public IRenameDialog
{
    Q_OBJECT

public:
    explicit RenameSingleDialog(QWidget *parent = 0);
    ~RenameSingleDialog();

    void setNames(const QStringList &names);

private slots:
    void on_btn_UpperAll_clicked();
    void on_btn_LowerAll_clicked();
    void on_btn_LowerExt_clicked();
    void on_btn_UpperExt_clicked();
    void accept();

private:
    Ui::RenameSingleDialog *ui;
};

#endif // RENAMESINGLEDIALOG_H
