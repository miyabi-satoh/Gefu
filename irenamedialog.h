#ifndef IRENAMEDIALOG_H
#define IRENAMEDIALOG_H

#include <QDialog>
#include <QDir>
#include "common.h"

class IRenameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit IRenameDialog(QWidget *parent = 0);

    const StringMap& renameMap() const { return m_RenameMap; }

    void setWorkingDirectory(const QString &path);
    virtual void setNames(const QFileInfoList &names) = 0;

signals:

public slots:

protected:
    QDir m_dir;
    StringMap m_RenameMap;
};

#endif // IRENAMEDIALOG_H
