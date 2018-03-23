#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "console.h"

#include "canboot_library/canboot.h"
#include "chai_library/chai.h"
#include "filedialog/filedialog.h"

#include <QMainWindow>
#include <QPushButton>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void slotInitButtonClicked();
    void slotChannelsActive(const bool setVar);    // Активировать кнопки
    void slotAddFreeChannelToCombo(int);               // Добавить канал
    void slotAddBusyChannelToCombo(int);

    void slotSendSignalCurrentBaud(int);
    void slotConnectButtonPressed(bool);

    void slotConnectionFailed(void);

    void slotSetProgressBarMaxValue(int);

signals:
    void signalPassBaud(int);

public:
    Console *can_console = nullptr;
    CANBoot *cboot = nullptr;

};

#endif // MAINWINDOW_H
