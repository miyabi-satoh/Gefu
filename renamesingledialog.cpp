#include "renamesingledialog.h"
#include "ui_renamesingledialog.h"
#include <QMessageBox>

RenameSingleDialog::RenameSingleDialog(QWidget *parent) :
    IRenameDialog(parent),
    ui(new Ui::RenameSingleDialog)
{
    ui->setupUi(this);

    ui->nameAfter->setFocus();
}

RenameSingleDialog::~RenameSingleDialog()
{
    delete ui;
}

void RenameSingleDialog::setNames(const QStringList &names)
{
    ui->nameBefore->setText(names.at(0));
    ui->nameAfter->setText(names.at(0));
}

void RenameSingleDialog::on_btn_UpperAll_clicked()
{
    ui->nameAfter->setText(ui->nameAfter->text().toUpper());
}

void RenameSingleDialog::on_btn_LowerAll_clicked()
{
    ui->nameAfter->setText(ui->nameAfter->text().toLower());
}

void RenameSingleDialog::on_btn_UpperExt_clicked()
{
    QFileInfo info(m_dir.absoluteFilePath(ui->nameAfter->text()));
    ui->nameAfter->setText(info.completeBaseName() + "." + info.suffix().toUpper());
}

void RenameSingleDialog::on_btn_LowerExt_clicked()
{
    QFileInfo info(m_dir.absoluteFilePath(ui->nameAfter->text()));
    ui->nameAfter->setText(info.completeBaseName() + "." + info.suffix().toLower());
}

void RenameSingleDialog::accept()
{
    if (ui->nameAfter->text() != ui->nameBefore->text()) {
        m_RenameMap.insert(m_dir.absoluteFilePath(ui->nameBefore->text()),
                           m_dir.absoluteFilePath(ui->nameAfter->text()));
    }

    IRenameDialog::accept();
}
