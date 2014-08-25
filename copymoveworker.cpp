#include "copymoveworker.h"
#include "overwritedialog.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <QThread>

CopyMoveWorker::CopyMoveWorker(QObject *parent) :
    IWorker(parent),
    m_CopyList(NULL),
    m_tgtDir(),
    m_CopyMap(),
    m_AskingMutex(),
    m_Asking(false),
    m_Move(false)
{
}

void CopyMoveWorker::operate()
{
    if (m_Move) {
        m_progressText->setText(tr("移動準備中..."));
    }
    else {
        m_progressText->setText(tr("コピー準備中..."));
    }

    foreach (const QFileInfo &info, *m_CopyList) {
        if (isStopRequested()) {
            emit canceled();
            return;
        }
        Listup(info.absoluteFilePath(), m_tgtDir);
    }

    bool ret;
    int successCount = 0;
    int skipCount = 0;
    int errorCount = 0;
    int errDelCount = 0;
    QString msg;
    QString copyMethod;
    QString alias;
    StringMap::iterator it;
    for (it = m_CopyMap.begin(); it != m_CopyMap.end(); it++) {
        if (isStopRequested()) {
            emit canceled();
            return;
        }
        QFileInfo srcInfo(it.key());
        QFileInfo tgtInfo(it.value());

        emit operation((m_Move ? tr("移動") : tr("コピー："))
                     + srcInfo.absoluteFilePath()
                     + tr(" -> ")
                     + tgtInfo.absoluteFilePath());

        if (srcInfo.isDir()) {
            if (tgtInfo.exists()) {
                emit success(tr("すでに存在"));
            }
            else if (QDir().mkdir(it.value())) {
                emit success("フォルダ作成");
            }
            else {
                emit error("フォルダの作成に失敗");
            }
            continue;
        }

        do {    // コピーをスキップするのにbreakを使うがためのdo-while
            if (tgtInfo.exists()) {
                // 解決方法を選択
                m_Asking = true;
                emit askOverWrite(&copyMethod,
                                  &alias,
                                  srcInfo.absoluteFilePath(),
                                  tgtInfo.absoluteFilePath());

                while (isAsking()) {
                    thread()->msleep(100);
                }

                // キャンセルされた？
                if (isStopRequested()) {
                    msg = tr("%1個のファイルをコピーしました。").arg(successCount);
                    if (skipCount > 0) {
                        msg += tr("%1個のファイルをスキップしました。").arg(skipCount);
                    }
                    if (errorCount > 0) {
                        msg += tr("%1個のファイルをコピーできませんでした。").arg(errorCount);
                    }
                    m_progressText->setText(msg);
                    emit canceled();
                    return;
                }

                if (copyMethod == "rbOverWrite") {
                    if (srcInfo.absoluteFilePath() == tgtInfo.absoluteFilePath()) {
                        emit success(tr("同一ファイルへの操作のためスキップ"));
                        skipCount++;
                        break;
                    }
                    QFile(tgtInfo.absoluteFilePath()).remove();
                }
                else if (copyMethod == "rbOverWriteIfNew") {
                    if (srcInfo.lastModified() <= tgtInfo.lastModified()) {
                        emit success(tr("古いファイルのためスキップ"));
                        skipCount++;
                        break;
                    }
                    if (srcInfo.absoluteFilePath() == tgtInfo.absoluteFilePath()) {
                        emit success(tr("同一ファイルへの操作のためスキップ"));
                        skipCount++;
                        break;
                    }
                    QFile(tgtInfo.absoluteFilePath()).remove();
                }
                else if (copyMethod == "rbAppendNumber") {
                    QString baseName = tgtInfo.baseName();
                    QString suffix = tgtInfo.completeSuffix();
                    for (int n = 1; ; n++) {
                        QString newName = baseName + tr("%1.").arg(n) + suffix;
                        tgtInfo.setFile(tgtInfo.absolutePath(), newName);
                        if (!tgtInfo.exists()) {
                            break;
                        }
                    }
                    emit operation(tr("=>") + tgtInfo.fileName() + tr("にリネーム"));
                }
                else if (copyMethod == "rbSkip") {
                    emit success(tr("スキップ"));
                    skipCount++;
                    break;
                }
                else if (copyMethod == "rbRename") {
                    tgtInfo.setFile(tgtInfo.absolutePath(), alias);
                    emit operation(tr("=>") + tgtInfo.fileName() + tr("にリネーム"));
                }
                else {
                    qDebug() << "Unknown method : " << copyMethod;
                    Q_ASSERT(false);
                }
            }

            ret = QFile::copy(srcInfo.absoluteFilePath(), tgtInfo.absoluteFilePath());
            if (ret) {
                successCount++;
                emit success(tr("成功"));
                if (m_Move && srcInfo.absoluteFilePath() != tgtInfo.absoluteFilePath()) {
                    ret = QFile(srcInfo.absoluteFilePath()).remove();
                    if (!ret) {
                        errDelCount++;
                        emit error("元ファイルの削除に失敗");
                    }
                }
            }
            else {
                errorCount++;
                emit error(tr("失敗"));
            }
        } while (0);

        msg = tr("%1ファイル成功。").arg(successCount);
        if (skipCount > 0) {
            msg += tr("%1ファイルスキップ。").arg(skipCount);
        }
        if (errorCount > 0) {
            msg += tr("%1ファイル失敗。").arg(errorCount);
        }
        if (errDelCount > 0) {
            msg += tr("%1ファイル削除失敗。").arg(errDelCount);
        }
        m_progressText->setText(msg);
    }

    if (m_Move) {
        // フォルダを削除する
        for (it = m_CopyMap.begin(); it != m_CopyMap.end(); it++) {
            if (QFileInfo(it.key()).isDir()) {
                QDir().rmpath(it.key());
            }
        }
    }

    emit finished();
}

void CopyMoveWorker::Listup(const QString &srcPath, const QString &tgtPath)
{
    if (isStopRequested()) {
        return;
    }

    QFileInfo info(srcPath);
    QDir tgtDir(tgtPath);

    m_CopyMap.insert(srcPath, tgtDir.absoluteFilePath(info.fileName()));
    qDebug() << srcPath << " -> " << tgtDir.absoluteFilePath(info.fileName());
    if (info.isDir()) {
        QDir srcDir(srcPath);
        foreach (QFileInfo info2, srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            Listup(info2.absoluteFilePath(),
                   tgtDir.absoluteFilePath(info.fileName()));
        }
    }
}
