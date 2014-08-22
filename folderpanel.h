#ifndef FOLDERPANEL_H
#define FOLDERPANEL_H

#include <QWidget>

namespace Ui {
class FolderPanel;
}

class FolderPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FolderPanel(QWidget *parent = 0);
    ~FolderPanel();

    const QString side() const;
    void setSide(const QString &side);

    void updateAppearance();

private:
    Ui::FolderPanel *ui;

private slots:
    void onStateChanged(int checkedFolders, int checkedFiles, quint64 totalSize);
    void on_locationField_editingFinished();
};

#endif // FOLDERPANEL_H
