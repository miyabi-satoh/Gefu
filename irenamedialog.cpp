#include "irenamedialog.h"

IRenameDialog::IRenameDialog(QWidget *parent) :
    QDialog(parent)
{
}

void IRenameDialog::setWorkingDirectory(const QString &path)
{
    m_dir.setPath(path);
}
