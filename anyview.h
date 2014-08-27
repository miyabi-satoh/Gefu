#ifndef ANYVIEW_H
#define ANYVIEW_H

#include <QFileInfo>
#include <QWidget>
class FolderPanel;
class SimpleTextView;
class SimpleImageView;

namespace Ui {
class AnyView;
}

class AnyView : public QWidget
{
    Q_OBJECT

public:
    enum {
        ViewFolder = 1,
        ViewText = 2,
        ViewImage = 3,
    };

    explicit AnyView(QWidget *parent = 0);
    ~AnyView();

    // action
    void changeView(int viewType);
    void setViewItem(const QFileInfo &info);

    // getter
    FolderPanel* folderPanel() const;
    SimpleImageView* imageView() const;
    SimpleTextView* textView() const;

private:
    Ui::AnyView *ui;

    // QWidget interface
protected:
    void focusInEvent(QFocusEvent *);
};

#endif // ANYVIEW_H
