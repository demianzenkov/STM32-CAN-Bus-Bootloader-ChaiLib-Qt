#include "logbrowserdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QCloseEvent>
#include <QKeyEvent>

LogBrowserDialog::LogBrowserDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);
    browser = new QTextBrowser(this);
    layout->addWidget(browser);
    resize(200, 400);
}


LogBrowserDialog::~LogBrowserDialog()
{

}


void LogBrowserDialog::outputMessage(QtMsgType type, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        browser->append(msg);
        break;

    case QtWarningMsg:
        browser->append(tr("— WARNING: %1").arg(msg));
        break;

    case QtCriticalMsg:
        browser->append(tr("— CRITICAL: %1").arg(msg));
        break;

    case QtFatalMsg:
        browser->append(tr("— FATAL: %1").arg(msg));
        break;
    }
}
