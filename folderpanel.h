#ifndef FOLDERPANEL_H
#define FOLDERPANEL_H

#include <QWidget>
#include <QDir>
#include <QTableWidget>

namespace Ui {
class FolderPanel;
}

class FolderPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FolderPanel(QWidget *parent = 0);
    ~FolderPanel();

    QTableWidget* fileTable();
    const QTableWidget* fileTable() const;

    QDir* dir() { return &m_dir; }
    const QDir* dir() const { return &m_dir; }

    void setCurrentFolder(const QString &path);

private:
    Ui::FolderPanel *ui;
    QDir m_dir;

    bool eventFilter(QObject *, QEvent *);

private slots:
    void on_fileTable_cellChanged(int row, int column);
//    void on_fileTable_doubleClicked(const QModelIndex &index);
    void on_locationField_editingFinished();
};

#endif // FOLDERPANEL_H
