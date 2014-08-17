#include "overwritedialog.h"
#include "ui_overwritedialog.h"
#include <QDir>
#include <QMessageBox>
#include <QDateTime>

OverWriteDialog::OverWriteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverWriteDialog),
    m_tgtPath()
{
    ui->setupUi(this);

    ui->tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    connect(ui->rbAppendNumber, SIGNAL(clicked()), this, SLOT(onRenameOrElse()));
    connect(ui->rbOverWrite, SIGNAL(clicked()), this, SLOT(onRenameOrElse()));
    connect(ui->rbOverWriteIfNew, SIGNAL(clicked()), this, SLOT(onRenameOrElse()));
    connect(ui->rbRename, SIGNAL(clicked()), this, SLOT(onRenameOrElse()));
    connect(ui->rbSkip, SIGNAL(clicked()), this, SLOT(onRenameOrElse()));

}

OverWriteDialog::~OverWriteDialog()
{
    delete ui;
}

void OverWriteDialog::setCopyMethod(int method)
{
    switch (method) {
    case OverWriteDialog::OverWrite:
        ui->rbOverWrite->setChecked(true);
        break;
    case OverWriteDialog::AppendNumber:
        ui->rbAppendNumber->setChecked(true);
        break;
    case OverWriteDialog::Skip:
        ui->rbSkip->setChecked(true);
        break;
    case OverWriteDialog::Rename:
        ui->rbRename->setChecked(true);
        ui->lineEdit->setFocus();
        break;
    default:
        ui->rbOverWriteIfNew->setChecked(true);
        break;
    }
    onRenameOrElse();
}

void OverWriteDialog::setSameMethodChecked(bool checked)
{
    ui->checkBox->setChecked(checked);
}

void OverWriteDialog::setFileInfo(const QString srcPath, const QString tgtPath)
{
    QFileInfo srcInfo(srcPath);
    QFileInfo tgtInfo(tgtPath);
    m_tgtPath = tgtPath;
    ui->lineEdit->setText(tgtInfo.fileName());

    QTableWidgetItem *iSrcSize = new QTableWidgetItem(tr("%1 Bytes").arg(srcInfo.size()));
    iSrcSize->setFlags(iSrcSize->flags() ^ Qt::ItemIsEditable);
    iSrcSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(0, 0, iSrcSize);

    QTableWidgetItem *iTgtSize = new QTableWidgetItem(tr("%1 Bytes").arg(tgtInfo.size()));
    iTgtSize->setFlags(iTgtSize->flags() ^ Qt::ItemIsEditable);
    iTgtSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(1, 0, iTgtSize);

    QTableWidgetItem *iSrcDate = new QTableWidgetItem(srcInfo.lastModified().toString("yy/MM/dd hh:mm"));
    iSrcDate->setFlags(iSrcDate->flags() ^ Qt::ItemIsEditable);
    iSrcDate->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(0, 1, iSrcDate);

    QTableWidgetItem *iTgtDate = new QTableWidgetItem(tgtInfo.lastModified().toString("yy/MM/dd hh:mm"));
    iTgtDate->setFlags(iTgtDate->flags() ^ Qt::ItemIsEditable);
    iTgtDate->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(1, 1, iTgtDate);
}

int OverWriteDialog::copyMethod()
{
    if (ui->rbAppendNumber->isChecked()) {
        return OverWriteDialog::AppendNumber;
    }
    if (ui->rbOverWrite->isChecked()) {
        return OverWriteDialog::OverWrite;
    }
    if (ui->rbOverWriteIfNew->isChecked()) {
        return OverWriteDialog::OverWriteIfNew;
    }
    if (ui->rbRename->isChecked()) {
        return OverWriteDialog::Rename;
    }
    return OverWriteDialog::Skip;
}

bool OverWriteDialog::isSameMethodChecked()
{
    return ui->checkBox->isChecked();
}

const QString OverWriteDialog::alias()
{
    return ui->lineEdit->text();
}

void OverWriteDialog::onRenameOrElse()
{
    if (ui->rbRename->isChecked()) {
        ui->lineEdit->setEnabled(true);
    }
    else {
        ui->lineEdit->setEnabled(false);
    }
}

void OverWriteDialog::accept()
{
    if (ui->rbRename->isChecked()) {
        QString newPath = QFileInfo(m_tgtPath).absoluteDir().absoluteFilePath(ui->lineEdit->text());
        if (QFileInfo(newPath).exists()) {
            QMessageBox::critical(
                        this,
                        tr("エラー"),
                        tr("すでに同名のファイルが存在しています。"));
            ui->lineEdit->setFocus();
            return;
        }
    }

    QDialog::accept();
}
