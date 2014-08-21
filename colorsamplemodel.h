#ifndef COLORSAMPLEMODEL_H
#define COLORSAMPLEMODEL_H

#include <QAbstractTableModel>
#include <QFont>
#include <QMap>

typedef QMap<QString, QColor> ColorMap;

class ColorSampleModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit ColorSampleModel(QObject *parent = 0);

    const QFont& font() { return m_font; }
    void setFont(const QFont &font);

    void setColorMap(ColorMap *colorMap) {
        m_colorMap = colorMap;
    }

    void update() {
        beginResetModel();
        endResetModel();
    }

private:
    ColorMap *m_colorMap;
    QFont m_font;

signals:

public slots:


    // QAbstractItemModel interface
public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
};


#endif // COLORSAMPLEMODEL_H
