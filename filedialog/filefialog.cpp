#include "filedialog.h"
#include <QFileDialog>


FileDialog::FileDialog(QWidget *parent) : QWidget(parent)
{

}

void FileDialog::slotOpenFileDialog()
{
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Open file"),
                "",
                tr("Bin Files (*.bin)")
                );

    QFile file(filename);

    if (!file.exists()) {
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    this->setFilePath(filename);

    emit signalFilePathIsSet(filename);
}


void FileDialog::setFilePath(QString path)
{
    this->filepath = path;
}

QString FileDialog::getFilePath(void)
{
    return (this->filepath);
}
