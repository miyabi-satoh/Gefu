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
    enum {
        Undefined       = 0x00,
        OverWrite       = 0x01,
        OverWriteIfNew  = 0x02,
        AppendNumber    = 0x04,
        Skip            = 0x08,
        Rename          = 0x10,
    };

    explicit OverWriteDialog(QWidget *parent = 0);
    ~OverWriteDialog();

    void setCopyMethod(int method);
    void setSameMethodChecked(bool checked);
    void setFileInfo(const QString srcPath, const QString tgtPath);

    int copyMethod();
    bool isSameMethodChecked();
    const QString alias();

private slots:
    void onRenameOrElse();
    void accept();

private:
    Ui::OverWriteDialog *ui;
    QString m_tgtPath;
};

#endif // OVERWRITEDIALOG_H
