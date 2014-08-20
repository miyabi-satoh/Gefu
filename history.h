#include <QStringList>

#ifndef HISTORY_H
#define HISTORY_H

#include <QVector>

class History
{
    typedef QVector<QString> Container;

public:
    History();

    void add(const QString &path);

    const QString& back();
    const QString& forward();
    const QString& current() const { return m_list[m_pos]; }

    bool isBegin() const    { return m_pos == 0; }
    bool isEnd() const      { return m_pos == m_list.size() - 1; }
    bool isEmpty() const    { return m_list.isEmpty(); }

    int size() const { return m_list.size(); }
    const QString& at(int n) const { return m_list.at(n); }
    void setAt(int n) { m_pos = n; }

private:
    Container m_list;
    int m_pos;
};

#endif // HISTORY_H
