#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include "colorsamplemodel.h"

#include <QDialog>
#include <QAbstractTableModel>
#include <QSettings>

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

    void saveAppearance(QSettings &settings);
    void loadAppearance(QSettings &settings, bool import);

private slots:
    void chooseFont();
    void setControlsEnabled(bool enabled);
    void setIgnoreExtDefault();
    void selectBoxColor();
    void selectViewColor();
    void selectViewerColor();
    void browseApp();
    void importAppearance();
    void exportAppearance();

    // QDialog interface
public slots:
    void accept();
};

#endif // PREFERENCEDIALOG_H
