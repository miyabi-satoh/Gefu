#include "renamemultidialog.h"
#include "ui_renamemultidialog.h"
#include <QFileIconProvider>
#include <QMessageBox>

RenameMultiDialog::RenameMultiDialog(QWidget *parent) :
    IRenameDialog(parent),
    ui(new Ui::RenameMultiDialog)
{
    ui->setupUi(this);

    // ヘッダーラベルを設定する
    QStringList labels;
    labels << tr("変更前") << tr("変更後");
    ui->tableWidget->setHorizontalHeaderLabels(labels);

    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
}

RenameMultiDialog::~RenameMultiDialog()
{
    delete ui;
}

void RenameMultiDialog::setNames(const QStringList &names)
{
    foreach (const QString &name, names) {
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);

        QTableWidgetItem *iBefore = new QTableWidgetItem(name);
        iBefore->setFlags(iBefore->flags() ^ Qt::ItemIsEditable);
        QFileInfo info(m_dir.absoluteFilePath(name));
        iBefore->setIcon(QFileIconProvider().icon(info));
        ui->tableWidget->setItem(row, 0, iBefore);

        QTableWidgetItem *iAfter = new QTableWidgetItem(name);
        ui->tableWidget->setItem(row, 1, iAfter);
    }
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
}

void RenameMultiDialog::on_btn_UpperAll_clicked()
{
    for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
        QTableWidgetItem *item = ui->tableWidget->item(n, 0);
        ui->tableWidget->item(n, 1)->setText(item->text().toUpper());
    }
}

void RenameMultiDialog::on_btn_LowerAll_clicked()
{
    for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
        QTableWidgetItem *item = ui->tableWidget->item(n, 0);
        ui->tableWidget->item(n, 1)->setText(item->text().toLower());
    }
}

void RenameMultiDialog::on_btn_UpperExt_clicked()
{
    for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
        QTableWidgetItem *item = ui->tableWidget->item(n, 0);
        QFileInfo info(m_dir.absoluteFilePath(item->text()));
        ui->tableWidget->item(n, 1)->setText(
                    info.completeBaseName() + "." + info.suffix().toUpper());
    }
}

void RenameMultiDialog::on_btn_LowerExt_clicked()
{
    for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
        QTableWidgetItem *item = ui->tableWidget->item(n, 0);
        QFileInfo info(m_dir.absoluteFilePath(item->text()));
        ui->tableWidget->item(n, 1)->setText(
                    info.completeBaseName() + "." + info.suffix().toLower());
    }
}

void RenameMultiDialog::on_replaceExec_clicked()
{
    for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
        QTableWidgetItem *item = ui->tableWidget->item(n, 0);
        QString name = item->text();

        if (!ui->replaceFrom->text().isEmpty()) {
            name.replace(ui->replaceFrom->text(), ui->replaceTo->text());
        }
        if (!ui->insertText->text().isEmpty()) {
            int pos = ui->insertPos->text().toInt();
            if (pos < 0) {
                pos = name.size() + pos + 1;
            }
            name.insert(pos, ui->insertText->text());
        }
        ui->tableWidget->item(n, 1)->setText(name);
    }
}

void RenameMultiDialog::on_numberExec_clicked()
{
    bool bOk;
    int nStart = ui->numberStart->text().toInt(&bOk);
    if (!bOk) {
        QMessageBox::critical(this,
                              tr("エラー"),
                              tr("連番開始番号が不正です。"));
        ui->numberStart->setFocus();
        return;
    }
    int nColumn = ui->numberStart->text().size();
    int nStep = ui->numberStep->text().toInt();
    for (int n = 0; n < ui->tableWidget->rowCount(); n++, nStart += nStep) {
        QTableWidgetItem *item = ui->tableWidget->item(n, 0);
        QString name = item->text();
        QFileInfo info(m_dir.absoluteFilePath(name));

        QString first = ui->numberFirst->text();
        first.replace("?B", info.completeBaseName());
        first.replace("?E", info.suffix());

        QString last = ui->numberLast->text();
        last.replace("?B", info.completeBaseName());
        last.replace("?E", info.suffix());

        QString fmt = "%0" + QString("%1").arg(nColumn) + "d";
        QString number;
        number.sprintf(fmt.toLatin1(), nStart);

        ui->tableWidget->item(n, 1)->setText(first + number + last);
    }
}

void RenameMultiDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button->text() == tr("Reset")) {
        for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
            QTableWidgetItem *item = ui->tableWidget->item(n, 0);
            ui->tableWidget->item(n, 1)->setText(item->text());
        }
    }
}

void RenameMultiDialog::accept()
{
    // 未入力チェック
    for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
        QTableWidgetItem *item = ui->tableWidget->item(n, 1);
        if (item->text().isEmpty()) {
            QMessageBox::critical(this,
                                  tr("エラー"),
                                  tr("変更後のファイル名が未入力です。"));
            ui->tableWidget->selectRow(n);
            return;
        }
    }

    for (int n = 0; n < ui->tableWidget->rowCount(); n++) {
        QTableWidgetItem *iBefore = ui->tableWidget->item(n, 0);
        QTableWidgetItem *iAfter = ui->tableWidget->item(n, 1);
        if (iBefore->text() != iAfter->text()) {
            m_RenameMap.insert(m_dir.absoluteFilePath(iBefore->text()),
                               m_dir.absoluteFilePath(iAfter->text()));
        }
    }

    IRenameDialog::accept();
}

