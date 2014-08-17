#ifndef IWORKER_H
#define IWORKER_H

#include <QLabel>
#include <QMutex>
#include <QObject>

class IWorker : public QObject
{
    Q_OBJECT
public:
    explicit IWorker(QObject *parent = 0);

    void setProgressText(QLabel *label) {
        m_progressText = label;
    }

    void requestStop() {
        QMutexLocker lock(&m_Mutex);
        m_stopRequested = true;
    }

protected:
    QLabel *m_progressText;

    bool isStopRequested() {
        QMutexLocker lock(&m_Mutex);
        return m_stopRequested;
    }

signals:
    void finished();
    void operation(const QString &msg);
    void success(const QString &msg);
    void error(const QString &msg);

public slots:
    virtual void operate() = 0;

private:
    QMutex m_Mutex;
    bool m_stopRequested;
};

#endif // IWORKER_H
