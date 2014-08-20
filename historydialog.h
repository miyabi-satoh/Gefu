#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include "history.h"

#include <QDialog>

namespace Ui {
class HistoryDialog;
}

class HistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HistoryDialog(QWidget *parent = 0);
    ~HistoryDialog();

    void setHistory(History *left, History *right);
    void setDefaultLeft(bool bLeft);

    int selectedIndex() const;
    const QString selectedSide() const;

private:
    Ui::HistoryDialog *ui;
    History *m_leftHistory;
    History *m_rightHistory;
    History *m_displaying;

private slots:
    void showLeftHistory();
    void showRightHistory();
    void showHistory();

    // QWidget interface
protected:
    void showEvent(QShowEvent *event);
};

#endif // HISTORYDIALOG_H
