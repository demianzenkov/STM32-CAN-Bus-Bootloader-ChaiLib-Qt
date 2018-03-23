#ifndef CANBOOT_H
#define CANBOOT_H

#include "console.h"
#include "bootfirmware.h"
#include "chai.h"

#include "filedialog/filedialog.h"

class CANBoot : public QObject
{
    Q_OBJECT

public:
    explicit CANBoot (QObject *parent = nullptr);
    ~CANBoot();

public:
    int lib_init();
    int get_binfo(void);
    int device_init();
    int get_chan(void);
    int init_channel(void);

    int getDumpByteLen(void);
    int getActiveChannel(void);

    void bootDumpData(void);

    int authFirstMessage(void);
    void getFirstKey(void);

    void encryptXOR(int *, int);
    void encryptReplace(int *);

    bool getConnectedFlag(void);
    void setConnectedFlag(bool);

private slots:
    void slotConnectionFailed(void);

public slots:
    void slotSetActiveChannel(int);
    void slotSetActiveBaud(int);
    void slotConnectButtonPressed(bool);
    void slotUploadButtonPressed(void);

    void slotFDOpenFileDialog(void);     // -> redirect signal from FD to MW
    void slotBFSetProgressBarMaxValue(int);   // -> redirect signal from BFirmware to MW

signals:
    void signalConsolePrint(const QString&);
    void signalDeviceActive(const bool);

    void signalFreeChannelFound(int);
    void signalBusyChannelFound(int);

    void signalProgressBarUpdate(int);

    void signalConnectionFailed(void);

    void signalFDOpenFileFialog(void);   // -> redirect signal from FD to MW

    void signalBFSetProgressBarMaxValue(int);   // -> redirect signal from BFirmware to MW


private:
    void set_bt(int baud);


private:
    canboard_t *binfo = nullptr;
    bootFirmware *bfirmware = nullptr;
    FileDialog *filedialog = nullptr;

    int active_channel;
    int active_baud;
    int bt0;
    int bt1;
    int dump_data[];
    int initkey;
    int firstkey[4];

    int encrypted_firstkey;
    bool connected_flag;
};

#endif // CANBOOT_H
