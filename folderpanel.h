#ifndef FOLDERPANEL_H
#define FOLDERPANEL_H

#include <QWidget>
class QLabel;
class LocationBox;
class FolderView;
class SearchBox;
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

    void initialize(MainWindow *mainWnd, bool left);

    // getter
    LocationBox* locationBox() const;
    FolderView* folderView() const;
    SearchBox* searchBox() const;
    QLabel* filterLabel() const;

    // setter
    void setNameFilters(const QString &filters = QString());

private:
    Ui::FolderPanel *ui;
    MainWindow *m_mainWnd;

    void showNameFilters();

public slots:
    void dataChange();
    void addBookmark();
    void itemFound();
    void itemNotFound();
    void searchItem(const QString& text);

    // QWidget interface
protected:
    void focusInEvent(QFocusEvent *);
};

#endif // FOLDERPANEL_H
