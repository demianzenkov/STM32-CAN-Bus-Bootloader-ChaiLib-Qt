#include "mainwindow.h"
#include "canboot.h"
#include "console.h"
#include "bootfirmware.h"


CANBoot::CANBoot(QObject *parent):
    QObject(parent),
    binfo(new canboard_t),
    bfirmware(new bootFirmware),
    filedialog(new FileDialog)
{

    connect(this, SIGNAL(signalFDOpenFileFialog()),
            this->filedialog, SLOT(slotOpenFileDialog()));

    connect(this->filedialog, SIGNAL(signalFilePathIsSet(QString)),
            this->bfirmware, SLOT(slotSetDumpArray(QString)));

    connect(this->bfirmware, SIGNAL(signalBFSetProgressBarMaxValue(int)),
            this, SLOT(slotBFSetProgressBarMaxValue(int)));

    connect(this, SIGNAL(signalConnectionFailed()),
            this, SLOT(slotConnectionFailed()));

    this->active_channel = -1;
    this->active_baud = -1;
    this->bt0 = 0;
    this->bt1 = 0;
    this->connected_flag = 0;

    this->initkey = 0x29923E1F;
}

CANBoot::~CANBoot()
{
}

int CANBoot::lib_init()
{
    int ret;
    if ((ret = CiInit()) < 0) // инициализцация библиотеки CHAI
    {
        QString tmpStr = "CHAI library initialization failed";
        emit signalConsolePrint(tmpStr);
        qDebug("CHAI library initialization failed\n");
        return (-1);
    }
    else
    {
        QString tmpStr = "CHAI library initialized\n";
        emit signalConsolePrint(tmpStr);

        int chver = CiGetLibVer();

        tmpStr = QString("Using CHAI %1.%2.%3\n").arg(int(VERMAJ(chver))).arg(int(VERMIN(chver))).arg(VERSUB(chver));
        emit signalConsolePrint(tmpStr);

        return 1;
    }
}

int CANBoot::device_init()
{
    if ((this->get_chan()) <= 0) {
        return -1;
    }

    else {
        emit signalDeviceActive(1);     // Аквтивировать объекты на панеи подключения
        return 1;
    }
}

/* ------------------------------------------------
 * int CANBoot::get_chan(void)
 * Return: статус выполнения функции
 *         > 0 - количество обнаруженных каналов
 *        <= 0 - ошибка
 ------------------------------------------------ */
int CANBoot::get_chan(void)
{
    int chFound = this->get_binfo();
    if (chFound > 0)    //
    {
        QString tmpStr = "Choose CAN channel and speed and press CONNECT\n";
        emit signalConsolePrint(tmpStr);
        return chFound;
    }
    else
    {
        QString tmpStr = "There are no CAN channels\nCheck device connection\n";
        emit signalConsolePrint(tmpStr);
        return -1;
    }
}

/* ------------------------------------------------
 * int CANBoot::binfo(void)
 * Return: количество обнаруженных каналов
 ------------------------------------------------ */
int CANBoot::get_binfo(void)
{

    _s16 i, j, ret;
    int cnt = 0;

    for (i = 0; i < CI_BRD_NUMS; i++)
    {
        this->binfo->brdnum = (_u8) i;
        ret = CiBoardInfo((this->binfo));
        if (ret < 0)
            continue;

        /* Print board name */
        QString tmpStr = QString("Device %1. %2 %3\n").arg(i).arg(this->binfo->name).arg(this->binfo->manufact);
        emit signalConsolePrint(tmpStr);

        // Проверка четырех каналов для каждого устройства
        for (j = 0; j < 4; j++)
        {
            if (this->binfo->chip[j] >= 0) // Если chip[j]<0 - канал отсутсвует
            {
                if ( CiOpen( (unsigned char) this->binfo->chip[j], CIO_CAN29 ) >= 0)
                {
                    CiClose( (_u8) binfo->chip[j]);
                    emit signalFreeChannelFound(this->binfo->chip[j]);
                }
                else
                {
                    emit signalBusyChannelFound(this->binfo->chip[j]);
                }
                cnt++;
            }
        }

    }
    return cnt;
}

int CANBoot::init_channel(void)
{
    int ret;
    if ((ret = CiOpen(this->active_channel, CIO_CAN29)) < 0)
    {
        //fprintf(stdout, "Error opening CAN channel %d, ret = %d\n", chan, ret);
        return ret;
    }

    if ((ret = CiSetBaud(this->active_channel, this->bt0, this->bt1)) < 0)
    {
        //fprintf(stdout, "can't set baud, ret = %d\n", ret);
        return ret;
    }

    // Установка фильтра на ID прошиваемого устройства
    if ((ret = CiSetFilter(this->active_channel, 0x0CFFFE46, 0xFFFFFFFF)) < 0)
    {
        //fprintf(stdout, "can't set hardware filter, ret = %d\n", ret);
        return ret;
    }

    return ret;
}

int CANBoot::getDumpByteLen(void)
{
    return (this->bfirmware->getArrayBytesLength());
}

int CANBoot::getActiveChannel(void)
{
    return (this->active_channel);
}

bool CANBoot::getConnectedFlag(void)
{
    return(this->connected_flag);
}

void CANBoot::setConnectedFlag(bool setVal)
{
    this->connected_flag = setVal;
}

/*
 * ---------------------------------------------
 *
 * Функция:          void bootDumpData(void)
 * Краткое описание: Загрузка прошивки
 *
 * ---------------------------------------------
*/
void CANBoot::bootDumpData(void)
{
    int ret, ret1;
    _u16 trqcnt_begin;
    canmsg_t tx;
    tx.id = 0xCFFFD40;		// ID
    tx.len = 8;
    tx.flags = 0b100;	// extended ID

    canwait_t cw;
    cw.chan = this->active_channel;
    cw.wflags = CI_WAIT_TR;
    int transmit_tout = 30 * CIQUE_DEFSIZE_TR;     // in millisecs

    ret = CiTrStat(this->active_channel, &trqcnt_begin);
    if (ret < 0)
        return;

    int tmp_tx_arr[8];

    for (int i = 0; i < this->bfirmware->getArrayMessagesLength(); i++)
    {
        /* Переворот для записи */
        for (int j = 0; j < 8; j++)
        {
            tmp_tx_arr[j] = this->bfirmware->getDumpArrayByte(i * 8 + 7 - j);
        }

        /* Шифрование сообщения */
        this->encryptXOR(tmp_tx_arr, this->encrypted_firstkey);

        /* Переворот для шифрования */
        this->encryptReplace(tmp_tx_arr);


        for (int j = 0; j < 8; j++)
        {
            tx.data[j] = tmp_tx_arr[j];
        }

        ret = CiTrStat(this->active_channel, &trqcnt_begin);

        ret = CiWrite(this->active_channel, &tx, 1);
        if (ret > 0)
        {
            emit signalProgressBarUpdate(i+1);
        }
        else
        {
            qDebug("Message cannot be sent");
            return;
        }
        //Sleep(2);		//WRITE TIMEOUT
    }

    ret1 = CiWaitEvent(&cw, 1, transmit_tout);
    if (ret1 <= 0)             // timeout or error
        return;
    emit signalConsolePrint("Firmware uploaded!\n");
}
/* --------------end bootDumpData()------------- */


/* ---------------------------------------------
 *
 * Функция:          void authFirstMessage(void)
 * Краткое описание: Отправка и прием сообщения
 * для инициализации подключенного устройства
 * Return: статус исполнения команды:
 *      <= 0 - ошибка
 *      > 0 - успешное выполнение
 * --------------------------------------------- */
int CANBoot::authFirstMessage(void)
{
    int ret = 0, ret1 = 0;

    canmsg_t init_tx;			// Структура на отправку
    init_tx.id = 0xCFFFD40;		// CAN ID драйвера
    init_tx.len = 8;
    init_tx.flags = 0b100;

    canwait_t cw;
    cw.chan = this->getActiveChannel();
    cw.wflags = CI_WAIT_RC | CI_WAIT_ER;

    canmsg_t rx;

    for (int i = 0; i < 8; i++)
    {
        init_tx.data[i] = this->bfirmware->getTXMessageByte(i);	// Отправка сообщения для инициализации
    }


    ret = CiWrite(this->active_channel, &init_tx, 1);
    if (ret < 0)
        return -1;

    ret = CiWaitEvent(&cw, 1, 10000);   // Ожидание ответа в течение 10 секунд

    if (ret > 0)								// Если ответ пришел
    {
        if (cw.rflags & CI_WAIT_RC) {
            ret1 = (int)CiRead(this->getActiveChannel(), &rx, 1);	// Прочитаем входящее сообщение
        }

        if (ret1 > 0) {							// Если сообщение удачно прочитано
            int initFlag = 1;
            for (int i = 0; i < 8; i++) {
                if (rx.data[i] != this->bfirmware->getRXMessageByte(i))	// Сравниваем входящее сообщение с заданным массивом
                    initFlag = 0;
            }
            if (initFlag == 1) {						// Если сообщения совпадают
                emit signalConsolePrint("Initialisation completed. Device connected.\n");
                this->setConnectedFlag(1);
                return 1;
            }
            else {										// Если сообщения не совпадают
                emit signalConsolePrint("Wrong answer from device. Initialisation aborted.\n");
                emit signalConnectionFailed();

                return -1;
            }
        }
        else  {			// Если произошел сбой во время чтения
            emit signalConsolePrint("Initialisation not completed.\n CiRead failed.\n");
            //emit signalConnectionFailed();

            return -1;
        }
    }
    else
    {			// Если ответ от устройства не пришел
        emit signalConsolePrint("Initialisation timeout. No answer from your device.\n");
        emit signalConnectionFailed();
        return -1;
    }
}

void CANBoot::getFirstKey(void)
{
    canmsg_t rx[50];

    uint16_t ret = 0, ret1 = 0;

    canwait_t cw;
    cw.chan = this->active_channel;
    cw.wflags = CI_WAIT_RC | CI_WAIT_ER;

    CiRcQueCancel(this->active_channel, &ret1);

    ret = CiWaitEvent(&cw, 1, 10000);
    uint16_t queCnt = 0;

    while (queCnt != 50)
    {
        ret1 = CiRcQueGetCnt(this->active_channel, &queCnt);
    }

    if (ret > 0)								// Если ответ пришел
    {
        if (cw.rflags & CI_WAIT_RC) {
            ret1 = CiRead(this->getActiveChannel(), (canmsg_t*)&rx, 50);	// Прочитаем входящее сообщение
        }

        if (ret1 > 0) {							// Если сообщение удачно прочитано
            for (int i = 0; i < 4; i++) {
                this->firstkey[i] = rx[14].data[4+i];
            }
            CiRcQueCancel(this->active_channel, &ret1);
            //qDebug("First key accepted.\n");
            emit signalConsolePrint("First key accepted.\n");
            /* decrypt received key */
            int tmp_key;
            tmp_key = ((this->firstkey[3] << 24) | (this->firstkey[2] << 16) | (this->firstkey[1] << 8) | (this->firstkey[0]));
            this->encrypted_firstkey = (tmp_key) ^ (this->initkey);
        }
    }
    else
    {
        emit signalConsolePrint("First key aborted.\n");
    }
}

void CANBoot::encryptXOR(int *arr_for_encr, int key)
{
    unsigned int tmp_arr_for_encr_1p[4];
    unsigned int tmp_arr_for_encr_2p[4];

    unsigned int tmp_arr_var_1p;
    unsigned int tmp_arr_var_2p;

    unsigned int tmp_encrypted_var_1p;
    unsigned int tmp_encrypted_var_2p;

    for (int i = 0; i < 4; i++)
    {
        tmp_arr_for_encr_1p[i] = arr_for_encr[i];
        tmp_arr_for_encr_2p[i] = arr_for_encr[i+4];
    }

    tmp_arr_var_1p = ((tmp_arr_for_encr_1p[0] << 24) | (tmp_arr_for_encr_1p[1] << 16) | (tmp_arr_for_encr_1p[2] << 8) | (tmp_arr_for_encr_1p[3]));
    tmp_arr_var_2p = ((tmp_arr_for_encr_2p[0] << 24) | (tmp_arr_for_encr_2p[1] << 16) | (tmp_arr_for_encr_2p[2] << 8) | (tmp_arr_for_encr_2p[3]));

    tmp_encrypted_var_1p = tmp_arr_var_1p ^ key;
    tmp_encrypted_var_2p = tmp_arr_var_2p ^ key;

    for (int i = 0; i < 4; i++)
    {
        arr_for_encr[i] = (tmp_encrypted_var_1p << i*8) >> (24);
        arr_for_encr[i+4] = (tmp_encrypted_var_2p << i * 8) >> (24);
    }
}

void CANBoot::encryptReplace(int *arr_for_encr)
{
    unsigned int tmp_arr[8];
    for (int i = 0; i < 8; i++)
    {
        tmp_arr[i] = arr_for_encr[i];
    }

    for (int i = 0; i < 8; i++)
    {
        switch (i)
        {
        case 0:
            arr_for_encr[i] = tmp_arr[7];
            break;
        case 1:
            arr_for_encr[i] = tmp_arr[3];
            break;
        case 2:
            arr_for_encr[i] = tmp_arr[6];
            break;
        case 3:
            arr_for_encr[i] = tmp_arr[2];
            break;
        case 4:
            arr_for_encr[i] = tmp_arr[5];
            break;
        case 5:
            arr_for_encr[i] = tmp_arr[1];
            break;
        case 6:
            arr_for_encr[i] = tmp_arr[4];
            break;
        case 7:
            arr_for_encr[i] = tmp_arr[0];
            break;

        }
    }
}

/* -------------- START SLOTS -------------- */


void CANBoot::slotSetActiveChannel(int active_ch)
{
    this->active_channel = active_ch;
}

void CANBoot::slotSetActiveBaud(int active_baud)
{
    this->active_baud = active_baud;
    this->set_bt(active_baud);
}


void CANBoot::slotConnectButtonPressed(bool checked)
{
    QString tmpStr;
    if (checked == 1)
    {
        if(this->init_channel() >= 0)
        {
            int ret;
            uint16_t  rxthres = 100;

            ret = CiWriteTout(this->getActiveChannel(), CI_CMD_SET, &rxthres);      //Установка таймаута на передачу

            if ((ret = CiStart(this->active_channel)) < 0)
            {
                tmpStr = QString("Conenction to channel %1 failed\n").arg(this->active_channel);
                emit signalConsolePrint(tmpStr);
            }
            else
            {
                tmpStr = QString("Attemping to connect to channel %1\n").arg(this->active_channel);
                emit signalConsolePrint(tmpStr);

                //this->getFirstKey();
                //this->setConnectedFlag(1);
                if (this->authFirstMessage() > 0)
                {
                    this->getFirstKey();
                }

                else
                {
                    tmpStr = QString("Connetion to channel %1 failed\n").arg(this->active_channel);
                    emit signalConsolePrint(tmpStr);
                    emit signalConnectionFailed();
                }
            }
        }
        else
        {
            tmpStr = QString("Connetion to channel %1 failed\n").arg(this->active_channel);
            emit signalConsolePrint(tmpStr);

            emit signalConnectionFailed();
        }

    }
    else
    {
        //CiStop(this->getActiveChannel());
        //CiClose(this->getActiveChannel());
        tmpStr = QString("Disconencted from channel %1\n").arg(this->active_channel);
        emit signalConsolePrint(tmpStr);
    }

}

void CANBoot::slotUploadButtonPressed(void)
{
    this->bootDumpData(); // Boot array
}

void CANBoot::slotFDOpenFileDialog(void)
{
    emit signalFDOpenFileFialog();
}

void CANBoot::slotBFSetProgressBarMaxValue(int val)
{
    emit this->signalBFSetProgressBarMaxValue(val);
}

void CANBoot::slotConnectionFailed()
{
    CiStop(this->getActiveChannel());
    CiClose(this->getActiveChannel());
}
/* -------------- END SLOTS -------------- */


void CANBoot::set_bt(int baud)
{
    switch (baud)
    {
    case 1000:
        this->bt0 = BCI_1M_bt0;
        this->bt1 = BCI_1M_bt1;
        break;
    case 800:
        this->bt0 = BCI_800K_bt0;
        this->bt1 = BCI_800K_bt1;
        break;
    case 500:
        this->bt0 = BCI_500K_bt0;
        this->bt1 = BCI_500K_bt1;
        break;
    case 250:
        this->bt0 = BCI_250K_bt0;
        this->bt1 = BCI_250K_bt1;
        break;
    case 125:
        this->bt0 = BCI_125K_bt0;
        this->bt1 = BCI_125K_bt1;
        break;
    case 100:
        this->bt0 = BCI_100K_bt0;
        this->bt1 = BCI_100K_bt1;
        break;
    case 50:
        this->bt0 = BCI_50K_bt0;
        this->bt1 = BCI_50K_bt1;
        break;
    case 20:
        this->bt0 = BCI_20K_bt0;
        this->bt1 = BCI_20K_bt1;
        break;
    case 10:
        this->bt0 = BCI_10K_bt0;
        this->bt1 = BCI_10K_bt1;
        break;
    }

}
