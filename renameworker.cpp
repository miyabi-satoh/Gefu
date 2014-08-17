#include "renameworker.h"
#include <QFile>
#include <QFileInfo>

RenameWorker::RenameWorker(QObject *parent) :
    IWorker(parent),
    m_RenameMap(NULL)
{
}

void RenameWorker::operate()
{
    m_progressText->setText(tr("名前を変更しています..."));
    StringMap::const_iterator it;
    bool ret;
    int successCount = 0;
    int errorCount = 0;
    QString msg;
    for (it = m_RenameMap->begin(); it != m_RenameMap->end(); it++) {
        if (isStopRequested()) {
            emit canceled();
            return;
        }

        emit operation(tr("名前変更：")
                     + QFileInfo(it.key()).fileName()
                     + tr(" -> ")
                     + QFileInfo(it.value()).fileName());
        ret = QFile::rename(it.key(), it.value());
        if (ret) {
            successCount++;
            emit success(tr("成功"));
        }
        else {
            errorCount++;
            emit error(tr("失敗"));
        }
        msg = tr("%1アイテム成功。").arg(successCount);
        if (errorCount > 0) {
            msg += tr("%1アイテム失敗。").arg(errorCount);
        }
        m_progressText->setText(msg);
    }

    emit finished();
}
