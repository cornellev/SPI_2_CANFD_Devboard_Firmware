#ifndef MCP251863_H
#define MCP251863_H

#include "pico/stdlib.h"
#include "hardware/spi.h"

//Constants and magic numbers
const uint MCP251863_BAUD_RATE = 160000000;

//Enums for various device modes/configs
enum MCP251863_writeMode_t { WRITE_NORM = 0, WRITE_CRC = 1, WRITE_SAFE = 2`};
enum MCP251863_readMode_t { READ_NORM = 0, READ_CRC = 1};
enum MCP251873_command_t : uint { 
    RESET = 0b000;
    READ = 0b0011;
    WRITE = 0b0010;
    READ_CRC = 0b1011;
    WRITE_CRC = 0b1010;
    WRITE_SAFE = 0b1100;
};
enum MCP251873_regAddr_t : uint {
    OSC = 0xE04,
    CRC = 0xE08,
    ECCCON = 0xE0C,
    ECCSTAT = 0xE10,
    DEVID = 0xE14,
    C1CON = 0x0;
    C1NBTCFG = 0x4;
    C1TDC = 0xC,
    C1TBC = 0x10,
    C1TSCON = 0x14,
    C1VEC = 0x18,
    C1INT = 0x1C,
    C1RXIF = 0x20,
    C1TXIF = 0x24,
    C1RXOVIF = 0x28,
    C1TXATIF = 0x2C,
    C1TXREQ = 0x30,
    C1TREC = 0x34,
    C1BDIAGx = 0x38, // 4 addresses between each
    C1TEFCON = 0x40,
    C1TEFSTA = 0x44,
    C1TEFUA = 0x48,
    C1TXQCON = 0x50,
    C1TXQSTA = 0x54,
    C1TXQUA = 0x58,
    C1FIFOCONx = 0x5C, // 12 addresses between each
    C1FIFOSTAx = 0x60, // 12 addresses between each
    C1FIFOUAx = 0x64, // 12 addresses between each
    C1FLTCONx = 1D0, // 4 addresses between each
    C1FLTOBJx = 0x1F8, // 8 addresses between each
    C1MASKx = 0x1F4 // 8 addresses between each
}

//Debug methods

//Main class
class MCP251863 {
    private:
        spi_inst_t* spi;

        uint readAddr(uint addr);
        uint writeAddr(uint addr, uint data);

    public:
        MCP251863_writeMode_t writeMode;
        MCP251863_readMode_t readMode;

        MCP251863(spi_int_t* spi_i);
        int reset();

        int configure_FIFO();

}

#endif