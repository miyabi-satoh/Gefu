#include "iworker.h"

IWorker::IWorker(QObject *parent) :
    QObject(parent),
    m_progressText(NULL),
    m_Mutex(),
    m_stopRequested(false)
{
}
