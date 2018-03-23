#include "console.h"

#include <QScrollBar>


Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    this->setReadOnly(true);
}

void Console::putData(const QString &qstr)
{
    QByteArray data = qstr.toUtf8();
    qDebug(data);
    insertPlainText(data);
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}
