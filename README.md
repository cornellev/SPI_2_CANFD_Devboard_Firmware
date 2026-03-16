# SPI_2_CANFD Bridge Firmware
RP2040 firmware to interface with the MCP256183 CANFD Controller + Transciever IC, targeted towards a custom development board developed by the CEV electrical team.

# rp2020-mcp251864
This is the main library used to interface with the MCP256183 IC via SPI, it defines a class than can be used to carry out basic functions on the IC, effectively abstracting it. 

# crc16
This is a fast lookup-table implementation of a 16 bit CRC using polynomial 0x8005. A communication interface is a critical system of the car and hence CRC even between the IC and MCU are needed to ensure reliability and reduce errors. 

# Custom Dev Board
<img width="1011" height="577" alt="image" src="https://github.com/user-attachments/assets/fd534313-6452-443d-b326-a6ed680c0307" />



