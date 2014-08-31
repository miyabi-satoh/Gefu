#include "deleteworker.h"
#include <QFileInfo>
#include <QDir>
#include <QDebug>

DeleteWorker::DeleteWorker(QObject *parent) :
    IWorker(parent),
    m_DeleteList(NULL),
    m_Targets()
{
}

void DeleteWorker::operate()
{
    m_progressText->setText(tr("削除準備中..."));

    foreach (const QFileInfo &info, *m_DeleteList) {
        if (isStopRequested()) {
            emit canceled();
            return;
        }
        Listup(info.absoluteFilePath());
    }

    bool ret;
    int successCount = 0;
    int errorCount = 0;
    QString msg;
    foreach (const QString &path, m_Targets) {
        if (isStopRequested()) {
            emit canceled();
            return;
        }

        emit operation(tr("削除：") + path);

        QFileInfo info(path);
        if (info.isDir()) {
            QDir dir(path);
            ret = dir.rmdir(path);
        }
        else {
            ret = QFile::remove(path);
        }

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

void DeleteWorker::Listup(const QString &path)
{
    if (isStopRequested()) {
        return;
    }

    if (QFileInfo(path).isDir()) {
        QDir dir(path);
        QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot |
                                               QDir::System |
                                               QDir::Hidden |
                                               QDir::AllDirs |
                                               QDir::Files,
                                               QDir::DirsFirst);
        foreach (const QFileInfo &info, list) {
            Listup(info.absoluteFilePath());
        }
    }

    m_Targets << path;
}
