#include "MCP251863.h"

int MCP251863::writeAddr() {

}

int MCP251863::readAddr(uint16_t addr, uint8_t* dst, size_t len) {
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
    uint16_t message = (cmd << 12) | (addr >> 4);

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
