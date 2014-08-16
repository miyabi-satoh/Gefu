#include "renamesingledialog.h"
#include "ui_renamesingledialog.h"

#include <QMessageBox>

RenameSingleDialog::RenameSingleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RenameSingleDialog)
{
    ui->setupUi(this);

    QSize size = this->size();
//    size.setWidth(260);
    size.setHeight(190);
    this->resize(size);

    ui->nameAfter->setFocus();
}

RenameSingleDialog::~RenameSingleDialog()
{
    delete ui;
}

void RenameSingleDialog::setName(const QString &name)
{
    ui->nameBefore->setText(name);
    ui->nameAfter->setText(name);
}

void RenameSingleDialog::setWorkingDirectory(const QString &dir)
{
    m_dir.setPath(dir);
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
        bool ret = QFile::rename(
                    m_dir.absoluteFilePath(ui->nameBefore->text()),
                    m_dir.absoluteFilePath(ui->nameAfter->text()));
        if (!ret) {
            QMessageBox::critical(
                        this,
                        tr("エラー"),
                        tr("ファイル名の変更に失敗しました。"));
            return;
        }
    }

    QDialog::accept();
}
