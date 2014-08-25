#include "common.h"
#include "overwritedialog.h"
#include "ui_overwritedialog.h"

#include <QDir>
#include <QMessageBox>
#include <QDateTime>
#include <QSettings>

OverWriteDialog::OverWriteDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OverWriteDialog),
    m_tgtPath()
{
    ui->setupUi(this);

    QHeaderView *header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
    header->setSectionResizeMode(0, QHeaderView::Stretch);

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

void OverWriteDialog::reset()
{
    QSettings settings;
    QString method = settings.value(IniKey_DefaultOnCopy).toString();
    QRadioButton *radio = findChild<QRadioButton*>(method);
    if (radio == NULL) {
        radio = ui->rbOverWriteIfNew;
    }
    radio->setChecked(true);

    ui->keepSetting->setChecked(false);
}

void OverWriteDialog::setFileInfo(const QString srcPath, const QString tgtPath)
{
    QFileInfo srcInfo(srcPath);
    QFileInfo tgtInfo(tgtPath);
    m_tgtPath = tgtPath;
    ui->lineEdit->setText(tgtInfo.fileName());

    QTableWidgetItem *iSrcName = new QTableWidgetItem(srcInfo.fileName());
    iSrcName->setFlags(iSrcName->flags() ^ Qt::ItemIsEditable);
    iSrcName->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setItem(0, 0, iSrcName);

    QTableWidgetItem *iTgtName = new QTableWidgetItem(tgtInfo.fileName());
    iTgtName->setFlags(iTgtName->flags() ^ Qt::ItemIsEditable);
    iTgtName->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->setItem(0, 0, iTgtName);

    QTableWidgetItem *iSrcSize = new QTableWidgetItem(tr("%1 Bytes").arg(srcInfo.size()));
    iSrcSize->setFlags(iSrcSize->flags() ^ Qt::ItemIsEditable);
    iSrcSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(0, 1, iSrcSize);

    QTableWidgetItem *iTgtSize = new QTableWidgetItem(tr("%1 Bytes").arg(tgtInfo.size()));
    iTgtSize->setFlags(iTgtSize->flags() ^ Qt::ItemIsEditable);
    iTgtSize->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(1, 1, iTgtSize);

    QTableWidgetItem *iSrcDate = new QTableWidgetItem(srcInfo.lastModified().toString("yy/MM/dd hh:mm"));
    iSrcDate->setFlags(iSrcDate->flags() ^ Qt::ItemIsEditable);
    iSrcDate->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(0, 2, iSrcDate);

    QTableWidgetItem *iTgtDate = new QTableWidgetItem(tgtInfo.lastModified().toString("yy/MM/dd hh:mm"));
    iTgtDate->setFlags(iTgtDate->flags() ^ Qt::ItemIsEditable);
    iTgtDate->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    ui->tableWidget->setItem(1, 2, iTgtDate);
}

QString OverWriteDialog::copyMethod() const
{
    return ui->method->checkedButton()->objectName();
}

bool OverWriteDialog::isKeepSetting() const
{
    return ui->keepSetting->isChecked();
}

const QString OverWriteDialog::alias() const
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
