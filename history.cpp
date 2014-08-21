#include "history.h"

History::History() :
    m_list(),
    m_pos(0)
{
}

void History::add(const QString &path)
{
    // 現在位置より後ろのデータは削除する
    if (!isEmpty() && !isEnd()) {
        m_list.resize(m_pos + 1);
    }
    // 現在位置と異なるパスであれば追加する
    if (isEmpty() || (m_list[m_pos] != path)) {
        m_list << path;
    }
    m_pos = m_list.size() - 1;
}

const QString &History::back()
{
    if (!isBegin()) {
        m_pos--;
    }
    return m_list[m_pos];
}

const QString &History::forward()
{
    if (!isEnd()) {
        m_pos++;
    }
    return m_list[m_pos];
}
