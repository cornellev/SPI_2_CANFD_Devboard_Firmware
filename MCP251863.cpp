#include "MCP251863.h"
#include <string.h>

MCP251863::MCP251863(spi_int_t* spi, uint CSPin) {
    this->spi = spi;
    this->CSPin = CSPin;
}

int MCP251863::writeAddr(uint16_t startAddr, uint8_t* data, size_t len) {
    MCP251873_command_t cmd;
    switch(writeMode) {
        case MCP251863_writeMode_t::WRITE_NORMAL:
            cmd = MCP251863_command_t::WRITE;
            break;
        case MCP251863_writeMode_t::WRITE_CRC:
            cmd = MCP251863_command_t::WRITE_CRC;
            break;
        case MCP251863_writeMode_t::WRITE_SAFE:
            cmd = MCP251863_command_t::WRITE_SAFE;
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
        case MCP251863_readMode_t::READ_NORMAL:
            cmd = MCP251863_command_t::READ;
            break;
        case MCP251863_readMode_t::READ_CRC:
            cmd = MCP251863_command_t::READ_CRC;
            break;
        default: 
            return 0;
    } 
    // form message CCCC-AAAAAAAAAAAA
    message[0] = (cmd << 4) | (startAddr >> 8);
    message[1] = startAddr >> 4;

    if (readMode == MCP251863_readMode_t::READ_CRC) {
        message[2] = (uint8_t)len;
    }

    // drive CS pin low
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 0);
    asm volatile("nop \n nop \n nop");

    // transmit message via SPI
    spi_write_blocking(spi, &message, 2);

    // write extra bits for len if CRC mode
    if (readMode == MCP251863_readMode_t::READ_CRC) {
        spi_write_blocking(spi, (&message)+2, 1);
    }

    // read out data
    spi_read_blocking(spi, 0, dst, len);
    
    // read out CRC
    if (readMode = MCP251863_readMode_t::READ_CRC) {
        spi_read_blocking(spi, 0, &crc, 2);
    }

    // drive CS pin high, ending read cycle
    asm volatile("nop \n nop \n nop");
    gpio_put(CSPin, 1);
    asm volatile("nop \n nop \n nop");

    // check CRC
    // later

    return 1;
}

int MCP251863::init() {
    return 1;

}

int MCP251863::reset() {
    MCP251863_command_t cmd = MCP251863_command_t::RESET;
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

int MCP251863::initGPFIFO(
    uint8_t fifoNum, MCP251863_fifoMode_t fifoMode,
    MCP251863_plSize_t plSize, uint8_t fSize,
    uint8_t prioNum, MCP251863_txfifofRetranMode_t retranMode
    MCP251863_fifoIntMode_t* intFlagArray, size_t intFlagSize
)
{
    uint8_t buff[4];
    MCP251863_regAddr_t addr = MCP251863_regAddr_t::C1FIFOCONx + 4*fifoNum;

    uint8_t intFlags = 0;
    for (int i=0; i<intFlagSize; i++) {
        intFlags |= intFlagArray[i];
    }

    //wait for reset bit to clear
    do {
        readAddr(addr+1, buff, 1);
    } while ((buff[0] >> 2) == 1);

    //set first byte, no RTR (CANFD used)
    buff[0] = 0b01000000 | intFlags | (fifoMode << 7);
    buff[1] = 0b00000000; 
    //assumes prioNum <= 32
    buff[2] = 0b00000000 | (retranMode << 5) | prioNum;
    //assumes fSize <= 32
    buff[3] = (plSize << 5) | fSize;

    writeAddr(addr, buff, 4);
    return 1;
}

int MCP251863::initTEFIFO(
    uint8_t fSize, 
    MCP251863_fifoIntMode_t* intFlagArray, size_t intFlagSize
)
{
    uint8_t buff[4];
    MCP251863_regAddr_t addr = MCP251863_regAddr_t::C1TEFCON;

    uint8_t intFlags = 0;
    for (int i=0; i<intFlagSize; i++) {
        intFlags |= intFlagArray[i];
    }

    //wait for reset bit to clear
    do {
        readAddr(addr + 1, buff, 1);
    } while ((buff[0] >> 2) == 1);

    //set bytes
    buff[0] = 0b00000000 | intFlags;
    buff[1] = 0b00000000;
    buff[2] = 0b00000000;
    //assumes fSize <= 32
    buff[3] = 0b00000000 | fSize;

    writeAddr(addr, buff, 4);
    return 1;
}

int MCP251863::initTXQ(
    MCP251863_plSize_t plSize, uint8_t fSize,
    uint8_t prioNum, MCP251863_txfifofRetranMode_t retranMode,
    MCP251863_fifoIntMode_t intFlagArray, size_t intFlagSize
) 
{
    uint8_t buff[4];
    MCP251863_regAddr_t addr = MCP251863_regAddr_t::C1TXQCON;

    uint8_t intFlags = 0;
    for (int i=0; i<intFlagSize; i++) {
        intFlags |= intFlagArray[i];
    }

    //wait for reset bit to clear
    do {
        readAddr(addr + 1, buff, 1);
    } while ((buff[0] >> 2) == 1);

    //set bytes
    buff[0] = 0b00000000 | intFlags;
    buff[1] = 0b00000000;
    //assumes prioNum <= 32
    buff[2] = 0b00000000 | (retranMode << 5) | prioNum;
    //assumes fSize <= 32
    buff[3] = (plSize << 5) | fSize;

    writeAddr(addr, buff, 4);
    return 1;
}

int MCP251863::setcontMode(MCP251873_conMode_t contMode) {
    MCP251863_regAddr_t addr = MCP251863_regAddr_t::C1CON;
    uint8_t buff0, buff1;

    //read current contMode 
    readAddr(addr+2, &buff0, 1)

    if ((buff0 >> 5) != MCP251863_contMode_t::CONFIG) {
        readAddr(addr+3, &buff1, 1);
        buff1 = (buff1 & 0b11111000) | MCP251863_contMode_t::CONFIG;
        writeAddr(addr+3, &buff1, 1);

        do {
            readAddr(addr+2, &buff0, 1);
        } while ((buff0 >> 5) != MCP251863_contMode_t::CONFIG);
    }

    //write intended contMode
    readAddr(addr+3, &buff1, 1);
    buff1 = (buff1 & 0b11111000) | contMode;
    writeAddr(addr+3, &buff1, 1);

    return 1;
}

int MCP251863::setTranMode(MCP251863_tranMode_t mode) {
    gpio_put(STBYPin, tranMode);
    return 1;
}
