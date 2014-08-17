#ifndef COPYWORKER_H
#define COPYWORKER_H

#include "common.h"
#include "iworker.h"

#include <QFileInfo>

class CopyWorker : public IWorker
{
    Q_OBJECT
public:
    explicit CopyWorker(QObject *parent = 0);

    void setCopyList(const QStringList *list) {
        m_CopyList = list;
    }
    void setTargetDir(const QString &path) {
        m_tgtDir = path;
    }

    void endAsking() {
        QMutexLocker lock(&m_AskingMutex);
        m_Asking = false;
    }

public slots:
    void operate();

signals:
    void askOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod, QString *alias,
                      const QString srcPath, const QString tgtPath);

private:
    const QStringList *m_CopyList;
    QString m_tgtDir;
    StringMap m_CopyMap;
    QMutex m_AskingMutex;
    bool m_Asking;

    bool isAsking() {
        QMutexLocker lock(&m_AskingMutex);
        return m_Asking;
    }

    void Listup(const QString &srcPath, const QString &tgtPath);
};

#endif // COPYWORKER_H
