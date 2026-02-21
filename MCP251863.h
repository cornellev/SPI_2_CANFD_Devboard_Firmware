#ifndef MCP251863_H
#define MCP251863_H

#include "pico/stdlib.h"
#include "hardware/spi.h"

//Constants and magic numbers
const uint MCP251863_BAUD_RATE = 160000000;

//Enums for various device modes/configs
enum class MCP251863_writeMode_t { WRITE_NORMAL = 0, WRITE_CRC = 1, WRITE_SAFE = 2`};
enum class MCP251863_readMode_t { READ_NORMAL = 0, READ_CRC = 1};
enum class MCP251873_command_t : uint8_t { 
    RESET = 0b000;
    READ = 0b0011;
    WRITE = 0b0010;
    READ_CRC = 0b1011;
    WRITE_CRC = 0b1010;
    WRITE_SAFE = 0b1100;
};

enum class MCP251873_contMode_t : uint8_t {
    CONFIG = 0b100,
    CANFD_NORMAL = 0b000,
    CAN2_NORMAL = 0b110, 
    SLEEP = 0b001,
    LISTEN_ONLY = 0b011,
    RESTR_OP = 0b111,
    INT_LOOP = 0b010,
    EXT_LOOP = 0b101
};

enum class MCP251863_tranMode_t : uint {
    STBY = 1,
    NORMAL = 0;
}

enum class MCP251863_txfifoRetranMode_t : uint8_t {
    NONE = 0b00;
    THREE = 0b01;
    UNLIM = 0b10;
}

enum class MCP251863_fifoIntMode_t : uint8_t {
    N_TFULL_REMPTY = 0b00000001, //fifo not full (TX), fifo not empty (RX)
    H_TFULL_REMPTY = 0b00000010, //fifo half full(TX), fifo half empty (RX)
    TFULL_REMPTY = 0b00000100, //fifo full (TX), fifo empty (RX),
    RXOV = 0b00001000, //fifo overflow (RX),
    TXAT = 0b00010000, //transmits exhausted,
}

enum class MCP251863_regAddr_t : uint16_t {
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
};

enum class MCP251863_fifoMode_t : uint8_t {
    TXMODE = 1,
    RXMODE = 0
};

enum class MCP251863_plSize_t : uint8_t {
    BYTES_8 = 0b000,
    BYTES_12 = 0b001,
    BYTES_16 = 0b010,
    BYTES_20 = 0b011,
    BYTES_24 = 0b100,
    BYTES_32 = 0b101,
    BYTES_48 = 0b110,
    BYTES_64 = 0b111,
}

//Debug methods

//Main class
class MCP251863 {
    private:
        spi_inst_t* spi;
        uint CSPin, STBYPin;
        uint setCS(uint state);
        uint readAddr(uint16_t startAddr, uint8_t* dst, size_t len);
        uint writeAddr(uint16_t startAddr, uint8_t* data, size_t len);

    public:
        MCP251863_writeMode_t writeMode;
        MCP251863_readMode_t readMode;
        MCP251863(spi_int_t* spi, uint CSPin, uint STBYPin);

        int init();
        int reset();

        int initGPFIFO(
            uint8_t fifoNum, MCP251863_fifoMode_t fifoMode, 
            MCP251863_plSize_t plSize, uint8_t fSize,
            uint8_t prioNum, MCP251863_txfifoRetranMode_t retranMode,
            MCP251863_fifoIntMode_t* intFlagArray, size_t intFlagSize
        );

        int initTEFIFO(
            uint8_t fSize, 
            MCP251863_fifoIntMode_t* intFlagArray, size_t intFlagSize
        );

        int initTXQ(
            MCP251863_plSize_t plSize, uint8_t fSize,
            uint8_t prioNum, MCP251863_txfifofRetranMode_t retranMode,
            MCP251863_fifoIntMode_t intFlagArray, size_t intFlagSize
        );

        int loadTXFIFO();
        int reqSendTXFIFO();

        int setContMode(MCP251863_contMode_t mode);
        int setTranMode(MCP251863_tranMode_t mode);


}

#endif