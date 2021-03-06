#ifndef OPERATIONDIALOG_H
#define OPERATIONDIALOG_H

#include "iworker.h"
#include <QDialog>

namespace Ui {
class OperationDialog;
}

class OperationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OperationDialog(QWidget *parent = 0);
    ~OperationDialog();

    void setWorker(IWorker *worker) {
        m_worker = worker;
    }
    void setAutoClose(bool yes);
    bool autoClose() const;

protected:
    void showEvent(QShowEvent *);

private:
    Ui::OperationDialog *ui;
    IWorker *m_worker;
    bool m_Error;

private slots:
    void onOperation(const QString &msg);
    void onSuccess(const QString &msg);
    void onError(const QString &msg);
    void onFinished();
    void onCanceled();
    void on_btnCloseCancel_clicked();
    void on_btnShowDetail_clicked();
};

#endif // OPERATIONDIALOG_H
