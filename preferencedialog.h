#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include "colorsamplemodel.h"

#include <QDialog>
#include <QAbstractTableModel>

namespace Ui {
class PreferenceDialog;
}

class PreferenceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferenceDialog(QWidget *parent = 0);
    ~PreferenceDialog();

private:
    Ui::PreferenceDialog *ui;
    ColorSampleModel m_model;
    ColorMap m_colorMap;

private slots:
    void changeFont();
    void setControlsEnabled(bool enabled);
    void selectBoxColor();
    void selectViewColor();

    // QDialog interface
public slots:
    void accept();
};

#endif // PREFERENCEDIALOG_H
