#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <QObject>
#include <QString>

class bootFirmware : public QObject
{
    Q_OBJECT

public:
    explicit bootFirmware (QObject *parent = nullptr);
    ~bootFirmware();


public:
    uint8_t getDumpArrayByte(int);

    uint8_t getTXMessageByte(int);
    uint8_t getRXMessageByte(int);

    int getArrayBytesLength(void);
    int getArrayMessagesLength(void);

    void initCRC32();
    void ProcessCRC(void* pData, int nLen);

signals:
    void signalBFSetProgressBarMaxValue(int);

public slots:
    void slotSetDumpArray(QString);

private:
    QByteArray dump_bytearray;
    QByteArray init_TX_message;
    QByteArray init_RX_message;

    int dump_byte_len;
    int dump_msg_len;

    uint32_t CRC_table[256];
    uint32_t CRC32;

};



#endif // FIRMWARE_H
