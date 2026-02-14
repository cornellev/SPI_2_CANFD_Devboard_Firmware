#include "MCP251863.h"

MCP251863::MCP251863(spi_int_t* spi, uint CSPin) {
    this->spi = spi;
    this->CSPin = CSPin;
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
    uint16_t message = (cmd << 12) | (startAddr >> 4);

    // drive CS pin low
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 0);
    asm volatile("nop \n nop \n nop");

    // transmit message via SPI
    spi_write_blocking(spi, &message, 2);

    // read out data
    spi_read_blocking(spi, 0, dst, len);

    // drive CS pin high, ending read cycle
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 1);
    asm volatile("nop \n nop \n nop");

    return 1;
}
