#ifndef DELETEWORKER_H
#define DELETEWORKER_H

#include "iworker.h"

class DeleteWorker : public IWorker
{
    Q_OBJECT
public:
    explicit DeleteWorker(QObject *parent = 0);

    void setDeleteList(const QStringList *list) {
        m_DeleteList = list;
    }

public slots:
    void operate();

private:
    const QStringList *m_DeleteList;
    QStringList m_Targets;

    void Listup(const QString &path);
};

#endif // DELETEWORKER_H
