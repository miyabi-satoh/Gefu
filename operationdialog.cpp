#include "common.h"
#include "mainwindow.h"
#include "operationdialog.h"
#include "ui_operationdialog.h"

#include <QThread>

OperationDialog::OperationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OperationDialog),
    m_worker(NULL),
    m_Error(false)
{
    ui->setupUi(this);
    resize(getMainWnd()->width() * 0.8, height());
}

OperationDialog::~OperationDialog()
{
    delete ui;
}

void OperationDialog::showEvent(QShowEvent *)
{
    ui->textEdit->setVisible(false);

    QThread *thread = new QThread();
    m_worker->moveToThread(thread);
    m_worker->setProgressText(ui->label);
    connect(thread, SIGNAL(started()), m_worker, SLOT(operate()));
    connect(m_worker, SIGNAL(finished()), thread, SLOT(quit()));

    connect(thread, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

    connect(m_worker, SIGNAL(operation(QString)), this, SLOT(onOperation(QString)));
    connect(m_worker, SIGNAL(success(QString)), this, SLOT(onSuccess(QString)));
    connect(m_worker, SIGNAL(error(QString)), this, SLOT(onError(QString)));
    connect(m_worker, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(m_worker, SIGNAL(canceled()), this, SLOT(onCanceled()));

    thread->start();
}

void OperationDialog::onOperation(const QString &msg)
{
    ui->textEdit->append(msg + " ... ");
}

void OperationDialog::onSuccess(const QString &msg)
{
    ui->textEdit->append("<font color='green'>" + msg + "</font>");
}

void OperationDialog::onError(const QString &msg)
{
    m_Error = true;
    ui->textEdit->append("<font color='red'><strong>" + msg + "</strong></font>");
}

void OperationDialog::onFinished()
{
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);
    ui->btnCloseCancel->setText(tr("閉じる"));

    ui->textEdit->append("");
    ui->textEdit->append(tr("完了"));
    if (!m_Error && ui->chkAutoClose->checkState() == Qt::Checked) {
        QDialog::accept();
    }
}

void OperationDialog::onCanceled()
{
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);
    ui->btnCloseCancel->setText(tr("閉じる"));

    ui->textEdit->append("");
    ui->textEdit->append(tr("操作は途中でキャンセルされました。"));
}

void OperationDialog::on_btnCloseCancel_clicked()
{
    if (ui->btnCloseCancel->text() == tr("閉じる")) {
        QDialog::accept();
    }
    else {
        m_worker->requestStop();
    }
}

void OperationDialog::on_btnShowDetail_clicked()
{
    if (ui->textEdit->isVisible()) {
        ui->textEdit->setVisible(false);
        ui->btnShowDetail->setText(tr("詳細を表示"));
    }
    else {
        ui->textEdit->setVisible(true);
        ui->btnShowDetail->setText(tr("詳細を隠す"));
    }
}
