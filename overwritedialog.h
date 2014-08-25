#ifndef OVERWRITEDIALOG_H
#define OVERWRITEDIALOG_H

#include <QDialog>
#include <QFileInfo>

namespace Ui {
class OverWriteDialog;
}

class OverWriteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OverWriteDialog(QWidget *parent = 0);
    ~OverWriteDialog();

    void reset();

    // setter
    void setFileInfo(const QString srcPath, const QString tgtPath);

    // getter
    QString copyMethod() const;
    bool isKeepSetting() const;
    const QString alias() const;

private slots:
    void onRenameOrElse();
    void accept();

private:
    Ui::OverWriteDialog *ui;
    QString m_tgtPath;
};

#endif // OVERWRITEDIALOG_H
