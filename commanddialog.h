#ifndef COMMANDDIALOG_H
#define COMMANDDIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class CommandDialog;
}

class CommandDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandDialog(QWidget *parent = 0);
    ~CommandDialog();

    void setCommandLine(const QString &cmdLine);
    void setWorkingDirectory(const QString &dir);

private:
    Ui::CommandDialog *ui;
    QProcess m_Process;

private slots:
    void accept();
};

#endif // COMMANDDIALOG_H
