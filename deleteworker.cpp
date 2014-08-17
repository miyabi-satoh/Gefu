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

    foreach (const QString &path, *m_DeleteList) {
        if (isStopRequested()) {
            break;
        }
        Listup(path);
    }

    if (!isStopRequested()) {
        int successCount = 0;
        int errorCount = 0;
        foreach (const QString &path, m_Targets) {
            if (isStopRequested()) {
                break;
            }

            emit operation(tr("削除：") + path);

            QFileInfo info(path);
            bool ret;
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

            QString msg;
            msg = tr("%1個のアイテムを削除しました。").arg(successCount);
            if (errorCount > 0) {
                msg += tr("%1個のアイテムを削除できませんでした。").arg(errorCount);
            }
            m_progressText->setText(msg);
        }
    }

    emit finished();
}

void DeleteWorker::Listup(const QString &path)
{
    qDebug() << tr("Listup: ") << path;

    if (isStopRequested()) {
        return;
    }

    QFileInfo info(path);

    if (info.isDir()) {
        QDir dir(path);
        foreach (QFileInfo info2, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            qDebug() << info2.fileName();
            Listup(info2.absoluteFilePath());
        }
    }
    qDebug() << "Targeting: " << path;
    m_Targets << path;
}
