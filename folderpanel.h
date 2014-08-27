#ifndef FOLDERPANEL_H
#define FOLDERPANEL_H

#include <QWidget>
class QLabel;
class LocationBox;
class FolderView;
class SearchBox;

namespace Ui {
class FolderPanel;
}

class FolderPanel : public QWidget
{
    Q_OBJECT

public:
    explicit FolderPanel(QWidget *parent = 0);
    ~FolderPanel();

    // getter
    LocationBox* locationBox() const;
    FolderView* folderView() const;
    SearchBox* serachBox() const;
    QLabel* filterLabel() const;

private:
    Ui::FolderPanel *ui;

    // QWidget interface
protected:
    void focusInEvent(QFocusEvent *);
};

#endif // FOLDERPANEL_H
