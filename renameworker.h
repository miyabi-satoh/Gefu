#ifndef RENAMEWORKER_H
#define RENAMEWORKER_H

#include "common.h"
#include "iworker.h"

class RenameWorker : public IWorker
{
    Q_OBJECT
public:
    explicit RenameWorker(QObject *parent = 0);

    void setRenameMap(const StringMap *map) {
        m_RenameMap = map;
    }

public slots:
    void operate();

private:
    const StringMap *m_RenameMap;
};

#endif // RENAMEWORKER_H
