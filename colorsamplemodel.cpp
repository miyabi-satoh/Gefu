#include "colorsamplemodel.h"

#include <QBrush>

ColorSampleModel::ColorSampleModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

void ColorSampleModel::setFont(const QFont &font)
{
    m_font = font;
}

int ColorSampleModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

int ColorSampleModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant ColorSampleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QString strText[3][2] = {
        { tr("通常"), tr("システム") },
        { tr("マーク"), tr("隠し属性") },
        { tr(""), tr("読取専用") }
    };

    switch (role) {
    case Qt::DisplayRole:
        return strText[index.row()][index.column()];

    case Qt::FontRole:
        return m_font;
        break;

    case Qt::BackgroundRole:
        switch (index.column()) {
        case 0:
            switch (index.row()) {
            case 0: return QBrush(m_colorMap->value("clrBgNormal"));
            case 1: return QBrush(m_colorMap->value("clrBgMark"));
            }
            break;

        case 1:
            switch (index.row()) {
            case 0:
            case 1:
            case 2:
                return QBrush(m_colorMap->value("clrBgNormal"));
            }
            break;
        }
        break;

    case Qt::ForegroundRole:
        switch (index.column()) {
        case 0:
            switch (index.row()) {
            case 0: return QBrush(m_colorMap->value("clrFgNormal"));
            case 1: return QBrush(m_colorMap->value("clrFgMark"));
            }
            break;

        case 1:
            switch (index.row()) {
            case 0: return QBrush(m_colorMap->value("clrFgSystem"));
            case 1: return QBrush(m_colorMap->value("clrFgHidden"));
            case 2: return QBrush(m_colorMap->value("clrFgReadonly"));
            }
            break;
        }
        break;
    }

    return QVariant();
}
