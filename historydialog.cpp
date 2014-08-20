#include "historydialog.h"
#include "ui_historydialog.h"

HistoryDialog::HistoryDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HistoryDialog),
    m_leftHistory(NULL),
    m_rightHistory(NULL),
    m_displaying(NULL)
{
    ui->setupUi(this);

}

HistoryDialog::~HistoryDialog()
{
    delete ui;
}

void HistoryDialog::setHistory(History *left, History *right)
{
    m_leftHistory = left;
    m_rightHistory = right;
}

void HistoryDialog::setDefaultLeft(bool bLeft)
{
    if (bLeft) {
        ui->leftPane->setChecked(true);
    }
    else {
        ui->rightPane->setChecked(true);
    }
}

int HistoryDialog::selectedIndex() const
{
    int row = ui->listWidget->currentIndex().row();
    return m_displaying->size() - row - 1;
}

const QString HistoryDialog::selectedSide() const
{
    if (ui->leftPane->isChecked()) {
        return QString("Left");
    }
    else {
        return QString("Right");
    }
}

void HistoryDialog::showLeftHistory()
{
    if (m_displaying != m_leftHistory) {
        m_displaying = m_leftHistory;
        showHistory();
    }

}

void HistoryDialog::showRightHistory()
{
    if (m_displaying != m_rightHistory) {
        m_displaying = m_rightHistory;
        showHistory();
    }
}

void HistoryDialog::showHistory()
{
    ui->listWidget->clear();
    for (int n = 0; n < m_displaying->size(); n++) {
        ui->listWidget->insertItem(0, m_displaying->at(n));
    }
    ui->listWidget->setCurrentRow(0);
}

void HistoryDialog::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    if (ui->leftPane->isChecked()) {
        showLeftHistory();
    }
    else {
        showRightHistory();
    }

    connect(ui->leftPane, SIGNAL(clicked()), this, SLOT(showLeftHistory()));
    connect(ui->rightPane, SIGNAL(clicked()), this, SLOT(showRightHistory()));
    connect(ui->listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));
}
