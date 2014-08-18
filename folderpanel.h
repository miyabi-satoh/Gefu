#ifndef FOLDERPANEL_H
#define FOLDERPANEL_H

#include <QWidget>
#include <QDir>
#include <QTableWidget>
#include <QFileIconProvider>
#include <QFileSystemWatcher>
#include "filetablewidget.h"
class MainWindow;

namespace Ui {
class FolderPanel;
}

class FolderPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FolderPanel(QWidget *parent = 0);
    ~FolderPanel();

    FileTableWidget* fileTable();
    const FileTableWidget* fileTable() const;

    QDir* dir() { return &m_dir; }
    const QDir* dir() const { return &m_dir; }

    void setCurrentFolder(const QString &path);
    void InstallWatcher();
    void UninstallWatcher();

    void beginUpdate()      { m_bUpdating = true; }
    void endUpdate()        { m_bUpdating = false; onUpdateMark(0, 0);}
    bool isUpdating() const { return m_bUpdating; }

private:
    Ui::FolderPanel *ui;
    QDir m_dir;
    QFileIconProvider m_IconFactory;
    QFileSystemWatcher *m_fsWatcher;
    bool m_bUpdating;

private slots:
    void onUpdateMark(int, int);
    void on_locationField_editingFinished();
    void on_directoryChanged(QString);
    void on_fileTable_itemSelectionChanged();
};

#endif // FOLDERPANEL_H
