#include "bootfirmware.h"

#include <QObject>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>

bootFirmware::bootFirmware(QObject *parent):
    QObject(parent)
{
    int tmp_TX_Message[] = { 0x98, 0xBA, 0xDC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF };
    int tmp_RX_Message[] = { 0xBB, 0x01, 0x3C, 0x11, 0x00, 0x00, 0x11, 0x49 };

    for (int j = 0; j < 8; j++)
    {
        this->init_TX_message[j] = tmp_TX_Message[j];
        this->init_RX_message[j] = tmp_RX_Message[j];
    }
}

bootFirmware::~bootFirmware()
{

}


uint8_t bootFirmware::getDumpArrayByte(int i)
{
    return (this->dump_bytearray[i]);
}

int bootFirmware::getArrayBytesLength(void)
{
    return (this->dump_byte_len);
}

int bootFirmware::getArrayMessagesLength(void)
{
    return (this->dump_msg_len);
}



uint8_t bootFirmware::getTXMessageByte(int i) {
    return (this->init_TX_message[i]);
}

uint8_t bootFirmware::getRXMessageByte(int i) {
    return (this->init_RX_message[i]);
}

/* ---- START SLOTS ---- */

void bootFirmware::slotSetDumpArray(QString filepath)
{
    QFile file(filepath);

    if (!file.exists()) {
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    this->dump_bytearray = file.readAll();
    this->dump_byte_len = this->dump_bytearray.length();
    this->dump_msg_len = (this->dump_byte_len) / 8;

    this->initCRC32();
    this->ProcessCRC(dump_bytearray.data(), dump_byte_len);

    emit (signalBFSetProgressBarMaxValue(dump_msg_len));
}

void bootFirmware::initCRC32(void)
{
    const unsigned CRC_POLY = 0xEDB88320;
    unsigned i, j, r;
    for (i = 0; i < 256; i++) {
        for (r = i, j = 8; j; j--)
            r = r & 1? (r >> 1) ^ CRC_POLY: r >> 1;
        this->CRC_table[i] = r;
    }
    this->CRC32 = 0;
}

void bootFirmware::ProcessCRC(void* pData, int nLen)
{
    const unsigned CRC_MASK = 0xD202EF8D;
    unsigned char* pdata = reinterpret_cast<unsigned char*>(pData);
    unsigned crc = this->CRC32;
    while (nLen--) {
        crc = this->CRC_table[static_cast<unsigned char>(crc) ^ *pdata++] ^ crc >> 8;
        crc ^= CRC_MASK;
    }
    this->CRC32 = crc;
}


/* ---- END SLOTS ---- */
