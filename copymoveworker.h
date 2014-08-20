#ifndef COPYWORKER_H
#define COPYWORKER_H

#include "common.h"
#include "iworker.h"

#include <QFileInfo>
#include <QMap>

class CopyMoveWorker : public IWorker
{
    Q_OBJECT
public:
    explicit CopyMoveWorker(QObject *parent = 0);

    void setCopyList(const QFileInfoList *list) {
        m_CopyList = list;
    }
    void setTargetDir(const QString &path) {
        m_tgtDir = path;
    }
    void setMoveMode(bool move) {
        m_Move = move;
    }

    void endAsking() {
        QMutexLocker lock(&m_AskingMutex);
        m_Asking = false;
    }

signals:
    void askOverWrite(bool *bOk, int *prevCopyMethod, int *copyMethod, QString *alias,
                      const QString &srcPath, const QString &tgtPath);

public slots:
    void operate();

private:
    const QFileInfoList *m_CopyList;
    QString m_tgtDir;
    StringMap m_CopyMap;
    QMutex m_AskingMutex;
    bool m_Asking;
    bool m_Move;

    bool isAsking() {
        QMutexLocker lock(&m_AskingMutex);
        return m_Asking;
    }

    void Listup(const QString &srcPath, const QString &tgtPath);
};

#endif // COPYWORKER_H
