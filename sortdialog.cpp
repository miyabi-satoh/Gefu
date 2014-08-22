#include "common.h"
#include "sortdialog.h"
#include "ui_sortdialog.h"
#include <QSettings>
#include <QDir>

SortDialog::SortDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SortDialog),
    m_RightOrLeft()
{
    ui->setupUi(this);
}

SortDialog::~SortDialog()
{
    delete ui;
}

void SortDialog::setRightOrLeft(const QString &s)
{
    QSettings settings;

    m_RightOrLeft = s;
    int sortBy = settings.value(m_RightOrLeft + slash + IniKey_SortBy).toInt();
    if (sortBy == SortByType) {
        ui->sortByType->setChecked(true);
    }
    else if (sortBy == SortBySize) {
        ui->sortBySize->setChecked(true);
    }
    else if (sortBy == SortByDate) {
        ui->sortByDate->setChecked(true);
    }
    else {
        ui->sortByName->setChecked(true);
    }

    int orderBy = settings.value(m_RightOrLeft + slash + IniKey_OrderBy).toInt();
    if (orderBy == OrderByDesc) {
        ui->orderDesc->setChecked(true);
    }
    else {
        ui->orderAsc->setChecked(true);
    }

    int putDirs = settings.value(m_RightOrLeft + slash + IniKey_PutDirs).toInt();
    if (putDirs == PutDirsDefault) {
        ui->dirsDefault->setChecked(true);
    }
    else if (putDirs == PutDirsLast) {
        ui->dirsLast->setChecked(true);
    }
    else {
        ui->dirsFirst->setChecked(true);
    }

    bool ignoreCase = settings.value(m_RightOrLeft + slash + IniKey_IgnoreCase).toBool();
    ui->ignoreCase->setChecked(ignoreCase);
}

void SortDialog::accept()
{
    QSettings settings;

    QString key = m_RightOrLeft + slash + IniKey_SortBy;
    if (ui->sortByDate->isChecked()) {
        settings.setValue(key, SortByDate);
    }
    else if (ui->sortByName->isChecked()) {
        settings.setValue(key, SortByName);
    }
    else if (ui->sortBySize->isChecked()) {
        settings.setValue(key, SortBySize);
    }
    else {
        settings.setValue(key, SortByType);
    }

    key = m_RightOrLeft + slash + IniKey_OrderBy;
    if (ui->orderAsc->isChecked()) {
        settings.setValue(key, OrderByAsc);
    }
    else {
        settings.setValue(key, OrderByDesc);
    }

    key = m_RightOrLeft + slash + IniKey_PutDirs;
    if (ui->dirsDefault->isChecked()) {
        settings.setValue(key, PutDirsDefault);
    }
    else if (ui->dirsFirst->isChecked()) {
        settings.setValue(key, PutDirsFirst);
    }
    else {
        settings.setValue(key, PutDirsLast);
    }

    key = m_RightOrLeft + slash + IniKey_IgnoreCase;
    settings.setValue(key, ui->ignoreCase->isChecked());

    QDialog::accept();
}
