#include "MCP251863.h"
#include <string.h>

MCP251863::MCP251863(spi_int_t* spi, uint CSPin) {
    this->spi = spi;
    this->CSPin = CSPin;
}

int MCP251863::crc16USB(uint8_t message*, uint16_t crc, size_t len) {
    // We dont do malloc in critical embedded systems!
    uint8_t buff[255];

    // copy messages into buffer
    memcpy(buff, message, len);
    memcpy(buff+len, &crc, 2);

    if ()
    // perform divisions
    for (int i=0; i<(8*len)-15; i++) {
        int k = i/8;
        buff[k] ^= (crc >> (8+(i%8)));
        buff[k+1] ^= 


    }

}

int MCP251863::writeAddr(uint16_t startAddr, uint8_t* data, size_t len) {
    MCP251873_command_t cmd;
    switch(writeMode) {
        case WRITE_NORM:
            cmd = WRITE;
            break;
        case WRITE_CRC:
            cmd = WRITE_CRC;
            break;
        case WRITE_SAFE:
            cmd = WRITE_SAFE;
        default: 
            return 0;
    } 
    // form message CCCC-AAAAAAAAAAAA
    uint16_t message = (cmd << 12) | (startAddr >> 4);

    // drive CS pin low
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 0);
    asm volatile("nop \n nop \n nop");

    // transmit message via SPI
    spi_write_blocking(spi, &message, 2);

    // write data
    spi_write_blocking(spi, data, len);

    // drive CS pin high, ending read cycle
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 1);
    asm volatile("nop \n nop \n nop");

    return 1;
}

int MCP251863::readAddr(uint16_t startAddr, uint8_t* dst, size_t len) {
    // set read command based on read mode
    MCP251873_command_t cmd;
    uint8_t message[3];
    uint16_t crc;
    switch(readMode) {
        case READ_NORM:
            cmd = READ;
            break;
        case READ_CRC:
            cmd = READ_CRC;
            break;
        default: 
            return 0;
    } 
    // form message CCCC-AAAAAAAAAAAA
    message[0] = (cmd << 4) | (startAddr >> 8);
    message[1] = startAddr >> 4;

    if (readMode == READ_CRC) {
        message[2] = (uint8_t)len;
    }

    // drive CS pin low
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 0);
    asm volatile("nop \n nop \n nop");

    // transmit message via SPI
    spi_write_blocking(spi, &message, 2);

    // write extra bits for len if CRC mode
    if (readMode == READ_CRC) {
        spi_write_blocking(spi, (&message)+2, 1);
    }

    // read out data
    spi_read_blocking(spi, 0, dst, len);
    
    // read out CRC
    if (readMode = READ_CRC) {
        spi_read_blocking(spi, 0, &crc, 2);
    }

    // drive CS pin high, ending read cycle
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 1);
    asm volatile("nop \n nop \n nop");

    // check CRC
    if ((readMode = READ_CRC) && ())

    return 1;
}

int MCP251863::reset() {
    MCP251863_command_t cmd = RESET;
    uint16_t message = cmd << 12;

    // drive CS pin low
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 0);
    asm volatile("nop \n nop \n nop");

    // transmit message via SPI
    spi_write_blocking(spi, &message, 2);

    // drive CS pin high, ending read cycle
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 1);
    asm volatile("nop \n nop \n nop");

    return 1;
}

int MCP251863::setOpMode(MCP251873_opMode_t) {
    MCP251863_command_t cmd;
}
