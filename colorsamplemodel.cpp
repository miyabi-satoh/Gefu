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
    return 5;
}

int ColorSampleModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QVariant ColorSampleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    const QString strText[5][3] = {
        { tr("通常"), tr("<DIR>"), tr("12/34/56 78:90") },
        { tr("マーク"), tr("123B"), tr("12/34/56 78:90") },
        { tr("システム"), tr("456KB"), tr("12/34/56 78:90") },
        { tr("隠し属性"), tr("789MB"), tr("12/34/56 78:90") },
        { tr("読取専用"), tr("10.2GB"), tr("12/34/56 78:90") }
    };

    switch (role) {
    case Qt::DisplayRole:
        return strText[index.row()][index.column()];

    case Qt::FontRole:
        return m_font;
        break;

    case Qt::BackgroundRole:
        if (index.row() == 1) {
            return QBrush(m_colorMap->value("clrBgMark"));
        }
        else {
            return QBrush(m_colorMap->value("clrBgNormal"));
        }
        break;

    case Qt::ForegroundRole:
        switch (index.row()) {
        case 0: return QBrush(m_colorMap->value("clrFgNormal"));
        case 1: return QBrush(m_colorMap->value("clrFgMark"));
        case 2: return QBrush(m_colorMap->value("clrFgSystem"));
        case 3: return QBrush(m_colorMap->value("clrFgHidden"));
        case 4: return QBrush(m_colorMap->value("clrFgReadonly"));
        break;
        }
    }

    return QVariant();
}
