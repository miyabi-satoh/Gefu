#include "common.h"
#include "bookmarkdialog.h"
#include "ui_bookmarkdialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QSettings>
//#include <QVector>

BookmarkDialog::BookmarkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookmarkDialog),
    m_isReadOnly(false)
{
    ui->setupUi(this);
    resize(parent->width() * 0.8, height());

    ui->tableWidget->setColumnCount(2);
}

BookmarkDialog::~BookmarkDialog()
{
    delete ui;
}

void BookmarkDialog::setEditMode(bool edit)
{
    ui->buttonAdd->setVisible(edit);
    ui->buttonDelete->setVisible(edit);
    ui->buttonDown->setVisible(edit);
    ui->buttonUp->setVisible(edit);

    m_isReadOnly = !edit;
    if (m_isReadOnly) {
        setWindowTitle(tr("ブックマークを開く"));
    }

    QSettings settings;
    for (int i = 0; ; i++) {
        QString name = settings.value(IniKey_BookmarkEntryName(i), "").toString();
        if (name.isEmpty()) {
            break;
        }

        QString path = settings.value(IniKey_BookmarkEntryPath(i)).toString();
        insertData(i, name, path);
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

int BookmarkDialog::selectedIndex() const
{
    return ui->tableWidget->currentRow();
}

void BookmarkDialog::insertData(int row, const QString &name, const QString &path)
{
    ui->tableWidget->insertRow(row);

    QTableWidgetItem *iName = new QTableWidgetItem(name);
    if (m_isReadOnly) {
        iName->setFlags(iName->flags() ^ Qt::ItemIsEditable);
    }
    ui->tableWidget->setItem(row, 0, iName);

    QTableWidgetItem *iPath = new QTableWidgetItem(path);
    iPath->setFlags(iPath->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(row, 1, iPath);
}

void BookmarkDialog::moveSelectedRows(bool up)
{
    qDebug() << "BookmarkDialog::moveSelectedRows();" << up;

    QList<QTableWidgetItem*> selected = ui->tableWidget->selectedItems();
    if (selected.isEmpty()) {
        qDebug() << "Not selected.";
        return;
    }

    QVector<QString> before;
    QVector<QString> after;

    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        QString name = ui->tableWidget->item(i, 0)->data(Qt::DisplayRole).toString();
        QString path = ui->tableWidget->item(i, 1)->data(Qt::DisplayRole).toString();
        before << name + "¥t" + path;
    }
    qDebug() << "set before" << before;

    after.resize(before.size());

    QVector<int> newRows;
    foreach (QTableWidgetItem *item, selected) {
        if (item->column() != 0) {
            int newRow = (up) ? item->row() - 1 : item->row() + 1;
            if (newRow < 0) {
                return;
            }
            else if (newRow >= ui->tableWidget->rowCount()) {
                return;
            }

            after[newRow] = before[item->row()];
            before[item->row()] = QString::null;
            newRows << newRow;
        }
    }

    qDebug() << "set after(1)" << after;
    int n = 0;
    for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
        if (!before[i].isEmpty()) {
            while (!after[n].isEmpty()) {
                n++;
            }
            after[n] = before[i];
        }
    }
    qDebug() << "set after(2)" << after;

    while (ui->tableWidget->rowCount() > 0) {
        ui->tableWidget->removeRow(0);
    }

    n = 0;
    foreach (const QString &val, after) {
        QStringList list = val.split("¥t");
        insertData(n, list[0], list[1]);
        n++;
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();

    foreach (int i, newRows) {
        for (n = 0; n < 2; n++) {
            QModelIndex index = ui->tableWidget->model()->index(i, n);
            ui->tableWidget->selectionModel()->select(index, QItemSelectionModel::Select);
        }
    }

}

void BookmarkDialog::accept()
{
    QSettings settings;
    QModelIndex index;
    int i;
    for (i = 0; i < ui->tableWidget->rowCount(); i++) {
        index = ui->tableWidget->model()->index(i, 0);
        QString name = ui->tableWidget->model()->data(index).toString();
        if (name.isEmpty()) {
            ui->tableWidget->setCurrentCell(i, 0);
            QMessageBox::critical(this, tr("エラー"), tr("名前が未入力です。"));
            return;
        }

        index = ui->tableWidget->model()->index(i, 1);
        QString path = ui->tableWidget->model()->data(index).toString();

        settings.setValue(IniKey_BookmarkEntryName(i), name);
        settings.setValue(IniKey_BookmarkEntryPath(i), path);
    }
    while (!settings.value(IniKey_BookmarkEntryName(i), "").toString().isEmpty()) {
        settings.remove(IniKey_BookmarkEntryName(i));
        settings.remove(IniKey_BookmarkEntryPath(i));
        i++;
    }

    QDialog::accept();
}

void BookmarkDialog::on_buttonDelete_clicked()
{
    foreach (const QTableWidgetItem *item, ui->tableWidget->selectedItems()) {
        if (item->column() == 0) {
            ui->tableWidget->removeRow(item->row());
        }
    }
}

void BookmarkDialog::on_buttonUp_clicked()
{
    moveSelectedRows(true);
}

void BookmarkDialog::on_buttonDown_clicked()
{
    moveSelectedRows(false);
}

void BookmarkDialog::on_buttonAdd_clicked()
{
    static QString initPath = QDir::homePath();

    QString path = QFileDialog::getExistingDirectory(
                this, tr("フォルダを選択"), initPath);
    if (!path.isEmpty()) {
        QFileInfo info(path);
        QString name(info.fileName());
        if (name.isEmpty()) {
            name = "root";
        }
        insertData(ui->tableWidget->rowCount(), name, path);
        initPath = path;

        ui->tableWidget->resizeColumnsToContents();
        ui->tableWidget->resizeRowsToContents();
    }
}
