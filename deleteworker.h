#ifndef DELETEWORKER_H
#define DELETEWORKER_H

#include "iworker.h"

#include <QFileInfoList>

class DeleteWorker : public IWorker
{
    Q_OBJECT
public:
    explicit DeleteWorker(QObject *parent = 0);

    void setDeleteList(const QFileInfoList *list) {
        m_DeleteList = list;
    }

public slots:
    void operate();

private:
    const QFileInfoList *m_DeleteList;
    QStringList m_Targets;

    void Listup(const QString &path);
};

#endif // DELETEWORKER_H
