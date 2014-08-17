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

    foreach (const QString &srcPath, *m_CopyList) {
        if (isStopRequested()) {
            emit canceled();
            return;
        }
        Listup(srcPath, m_tgtDir);
    }

    bool ret;
    int copyMethod = OverWriteDialog::Undefined;
    int prevCopyMethod = OverWriteDialog::Undefined;
    int successCount = 0;
    int skipCount = 0;
    int errorCount = 0;
    int errDelCount = 0;
    QString msg;
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
                if (copyMethod == OverWriteDialog::Undefined ||
                        copyMethod == OverWriteDialog::Rename)
                {
                    bool bOk;
                    m_Asking = true;
                    emit askOverWrite(&bOk, &prevCopyMethod, &copyMethod, &alias,
                                      srcInfo.absoluteFilePath(),
                                      tgtInfo.absoluteFilePath());
                    while (isAsking()) {
                        this->thread()->msleep(100);
                    }

                    if (!bOk) {
                        requestStop();
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
                }

                if (prevCopyMethod == OverWriteDialog::OverWrite) {
                    QFile(tgtInfo.absoluteFilePath()).remove();
                }
                else if (prevCopyMethod == OverWriteDialog::OverWriteIfNew) {
                    if (srcInfo.lastModified() <= tgtInfo.lastModified()) {
                        emit success(tr("スキップ"));
                        skipCount++;
                        break;
                    }
                    QFile(tgtInfo.absoluteFilePath()).remove();
                }
                else if (prevCopyMethod == OverWriteDialog::AppendNumber) {
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
                else if (prevCopyMethod == OverWriteDialog::Skip) {
                    emit success(tr("スキップ"));
                    skipCount++;
                    break;
                }
                else if (prevCopyMethod == OverWriteDialog::Rename) {
                    tgtInfo.setFile(tgtInfo.absolutePath(), alias);
                    emit operation(tr("=>") + tgtInfo.fileName() + tr("にリネーム"));
                }
            }

            ret = QFile::copy(srcInfo.absoluteFilePath(), tgtInfo.absoluteFilePath());
            if (ret) {
                successCount++;
                emit success(tr("成功"));
                if (m_Move) {
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
//    qDebug() << srcPath << " -> " << tgtDir.absoluteFilePath(info.fileName());
    if (info.isDir()) {
        QDir srcDir(srcPath);
        foreach (QFileInfo info2, srcDir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            Listup(info2.absoluteFilePath(),
                   tgtDir.absoluteFilePath(info.fileName()));
        }
    }
}
