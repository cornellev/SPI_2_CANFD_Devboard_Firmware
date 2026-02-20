#include "crc16.h"

uint16_t CRC16_USB(uint8_t *data, size_t len) {
    //initial crc value 0xFFFF
    uin16_t crc = 0xFFFF;
    for (int i=0; i<len; i++) {
        //calculate dividend
        uint8_t pos = (uint8_t)((crc ^ (data[i])) & 0xFF);
        //use LUT to get remainder, shift out old MSB and XOR in remainder
        crc = (uint16_t)((crc >> 8) ^ CRC16_USB_RLUT[pos]);
    }
    return crc ^ 0xFFFF;
}

