#include <QMetaType>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "console.h"
#include "canboot_library/canboot.h"
#include "filedialog/filedialog.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    can_console(new Console),
    cboot(new CANBoot)
{   
    ui->setupUi(this);
    ui->upd_progressBar->setValue(0);

    /* Нажатие на кнопку Find Devices */
    connect(ui->find_pushButton, SIGNAL (clicked(bool)),
            this, SLOT (slotInitButtonClicked()));

    /* Отправка сообщения в консольное окно */
    connect(cboot, SIGNAL(signalConsolePrint(const QString&)),
            can_console, SLOT (putData(const QString&)));

    /* Найдены доступные устройства */
    connect(cboot, SIGNAL(signalDeviceActive(const bool)),
            this, SLOT(slotChannelsActive(const bool)));

    /* Найден свободный канал */
    connect(cboot, SIGNAL(signalFreeChannelFound(int)),
            this, SLOT(slotAddFreeChannelToCombo(int)));
    /* Найден занятой канал */
    connect(cboot, SIGNAL(signalBusyChannelFound(int)),
            this, SLOT(slotAddBusyChannelToCombo(int)));

    /* Выбран свободный канал */
    connect(ui->channel_comboBox, SIGNAL(currentIndexChanged(int)),
            cboot, SLOT(slotSetActiveChannel(int)));

    /* Выбрана скорость подключения */
    connect(ui->speed_comboBox, SIGNAL(activated(int)),
            this, SLOT(slotSendSignalCurrentBaud(int)));
    connect(this, SIGNAL(signalPassBaud(int)),
            cboot, SLOT(slotSetActiveBaud(int)));

    /* Нажатие на кнопку Сonnect */
    connect(ui->connect_pushButton, SIGNAL (toggled(bool)),
            cboot, SLOT (slotConnectButtonPressed(bool)));

    connect(ui->connect_pushButton, SIGNAL (toggled(bool)),
            this, SLOT (slotConnectButtonPressed(bool)));

    /* Нажатие на кнопку Upload */
    connect(ui->upd_pushButton, SIGNAL (clicked(bool)),
            cboot, SLOT (slotUploadButtonPressed()));

    /* Обновление ProgressBar */
    connect(cboot, SIGNAL (signalProgressBarUpdate(int)),
            ui->upd_progressBar, SLOT (setValue(int)));

    /* Connection failed */
    connect(cboot, SIGNAL (signalConnectionFailed(void)),
            this, SLOT (slotConnectionFailed(void)));

    /* File Dialog */
    connect(ui->open_pushButton, SIGNAL(clicked(bool)),
            cboot, SLOT(slotFDOpenFileDialog(void)));
    connect(cboot, SIGNAL(signalBFSetProgressBarMaxValue(int)),
            this, SLOT(slotSetProgressBarMaxValue(int)));



    /* Add console window to UI */
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(can_console);
    ui->console_groupBox->setLayout(vbox);

    cboot->lib_init();
}

MainWindow::~MainWindow()
{
    CiStop(this->cboot->getActiveChannel());
    CiClose(this->cboot->getActiveChannel());
    delete ui;
}

/*          START SLOTS           */

void MainWindow::slotInitButtonClicked ()
{
    ui->channel_comboBox->clear();
    cboot->device_init();
}

void MainWindow::slotChannelsActive(const bool setVar)
{
    ui->channel_comboBox->setEnabled(setVar);
    ui->speed_comboBox->setEnabled(setVar);
}

void MainWindow::slotAddFreeChannelToCombo(int ch)
{
    QString new_channel = QString("%1-free").arg(ch);
    ui->channel_comboBox->addItem(new_channel);
}

void MainWindow::slotAddBusyChannelToCombo(int ch)
{
    QString new_channel = QString("%1-busy").arg(ch);
    ui->channel_comboBox->addItem(new_channel);
}

void MainWindow::slotSendSignalCurrentBaud(int)
{
    QString ch_baud = ui->speed_comboBox->itemText(ui->speed_comboBox->currentIndex());
    int cur_baud = ch_baud.toInt();
    ui->connect_pushButton->setEnabled(1);
    emit signalPassBaud(cur_baud);
}

void MainWindow::slotConnectButtonPressed(bool checked)
{
    if ((checked == 1) and (cboot->getConnectedFlag() == 1))
    {
        ui->find_pushButton->setEnabled(0);
        ui->channel_comboBox->setEnabled(0);
        ui->speed_comboBox->setEnabled(0);
        ui->upd_pushButton->setEnabled(1);
        ui->upd_progressBar->setEnabled(1);
        ui->connect_pushButton->setText("DISCONNECT");
    }
    else
    {
        ui->find_pushButton->setEnabled(1);
        ui->channel_comboBox->setEnabled(1);
        ui->speed_comboBox->setEnabled(1);

        ui->upd_pushButton->setEnabled(0);
        ui->upd_progressBar->setEnabled(0);
        ui->upd_progressBar->setValue(0);

        ui->connect_pushButton->setEnabled(0);
        ui->connect_pushButton->setText("CONNECT");
    }
}

void MainWindow::slotConnectionFailed(void)
{
    ui->find_pushButton->setEnabled(1);
    ui->channel_comboBox->setEnabled(1);
    ui->speed_comboBox->setEnabled(1);


    ui->connect_pushButton->setEnabled(1);
    ui->connect_pushButton->setChecked(0);
    ui->connect_pushButton->setText("CONNECT");

    ui->upd_pushButton->setEnabled(0);
    ui->upd_progressBar->setEnabled(0);

    CiStop(this->cboot->getActiveChannel());
    CiClose(this->cboot->getActiveChannel());

    this->cboot->setConnectedFlag(0);

}

void MainWindow::slotSetProgressBarMaxValue(int val)
{
    ui->upd_progressBar->setMaximum(val);
}

/*          END SLOTS           */
