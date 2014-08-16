#include "commanddialog.h"
#include "ui_commanddialog.h"
#include <QPushButton>
#include <QDebug>

CommandDialog::CommandDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CommandDialog),
    m_Process(this)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("実行"));
}

CommandDialog::~CommandDialog()
{
    delete ui;
}

void CommandDialog::setCommandLine(const QString &cmdLine)
{
    ui->lineEdit->setText(cmdLine);
}

void CommandDialog::setWorkingDirectory(const QString &dir)
{
    m_Process.setWorkingDirectory(dir);
}

void CommandDialog::accept()
{
    if (!ui->lineEdit->text().isEmpty()) {
        m_Process.startDetached(ui->lineEdit->text());
    }
    QDialog::accept();
}
