#ifndef FILEDIALOG_H
#define FILEDIALOG_H

#include <QWidget>

class FileDialog : public QWidget
{
    Q_OBJECT
public:
    explicit FileDialog(QWidget *parent = nullptr);

signals:
    void signalFilePathIsSet(QString);

public slots:
    void slotOpenFileDialog(void);

private:
    QString filepath;

public:
    void setFilePath(QString);
    QString getFilePath(void);

};

#endif // FILEDIALOG_H
