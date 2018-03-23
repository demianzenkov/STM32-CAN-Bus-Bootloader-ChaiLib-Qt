#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QTextBrowser;
class QPushButton;

class LogBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    LogBrowserDialog(QWidget *parent = 0);
    ~LogBrowserDialog();

public slots:
    void outputMessage( QtMsgType type, const QString &msg );

protected:
    QTextBrowser *browser;
};

#endif // DIALOG_H
