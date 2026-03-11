#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "pinout.h"
#include "rp2040-mcp251863/mcp251863.h"

#include "pico-ssd1306/ssd1306.h"
#include "pico-ssd1306/textRenderer/TextRenderer.h"
#include "hardware/i2c.h"

#include "hardware/irq.h"

volatile int n_messages;
volatile bool message_waiting = false;

void increment_message(uint gpio, uint32_t events) {
    if (gpio == CAN_nINT1) {
        if (events & GPIO_IRQ_EDGE_FALL) {
            n_messages++;
            message_waiting = true;
            gpio_put(17, 1);
            gpio_acknowledge_irq(gpio, GPIO_IRQ_EDGE_FALL);
        }
    }
}

int main() {
    stdio_init_all();

    //init I2C
    i2c_init(i2c1, 1000000);

    gpio_init(17);
    gpio_set_dir(17, GPIO_OUT);
    gpio_put(17, 1);

    gpio_set_function(DIS_SDA, GPIO_FUNC_I2C);
    gpio_set_function(DIS_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(DIS_SDA);
    gpio_pull_up(DIS_SCL);

    //delay for ssd1306 to start
    sleep_ms(250);

    pico_ssd1306::SSD1306 display = pico_ssd1306::SSD1306(i2c1, 0x3C, pico_ssd1306::Size::W128xH32);
    display.setOrientation(0);

    //init spi and spi related pins
    spi_inst_t *spi = spi0;
    spi_init(spi, 1000);

    gpio_init(CAN_nCS);
    gpio_set_dir(CAN_nCS, GPIO_OUT);
    gpio_put(CAN_nCS, 1);

    gpio_set_function(CAN_SDO, GPIO_FUNC_SPI);
    gpio_set_function(CAN_SDI, GPIO_FUNC_SPI);
    gpio_set_function(CAN_SCK, GPIO_FUNC_SPI);

    //init pins related to MCP2518
    gpio_init(CAN_STBY);
    gpio_set_dir(CAN_STBY, GPIO_OUT);

    gpio_init(CAN_nINT0);
    gpio_set_dir(CAN_nINT0, GPIO_IN);
    gpio_pull_up(CAN_nINT0);

    gpio_init(CAN_nINT1);
    gpio_set_dir(CAN_nINT1, GPIO_IN);
    gpio_pull_up(CAN_nINT1);

    //interrupts for RX
    gpio_set_irq_enabled_with_callback(CAN_nINT1, GPIO_IRQ_EDGE_FALL, true, &increment_message);            
            

    MCP251863 bridge = MCP251863(spi, CAN_nCS, CAN_STBY);

    //code for setup
    bridge.writeMode = WM_MCP_NORM;
    bridge.readMode = RM_MCP_NORM;

    bridge.reset();
    bridge.setContMode(CMODE_MCP_CONF);
    bridge.setTranMode(TMODE_MCP_NORM);

    bridge.setPinMode(IO_MCP_INT0, IOMODE_MCP_INT);
    bridge.setPinMode(IO_MCP_INT1, IOMODE_MCP_INT);

    fifo_int_mode_MCP251863_t flagArray[] = {
        FIFO_INT_MCP_NFNE, FIFO_INT_MCP_OVFL
    };

    int_en_MCP251863_t intEnArray[] = {
        INT_EN_MCP_RXIE, INT_EN_MCP_RXOVIE
    };

    bridge.setInterrupts(intEnArray, 2);

    int MODE = 0;
    //0 - sender
    //1 - receiver

    //for TX mode
    if (MODE == 0) {
        bridge.initGPFIFO(1, FIFO_MODE_MCP_TX, PL_SIZE_MCP_8, 8, 0b11111, TXRET_MCP_THREE, flagArray, 2);
    }
    else {
        bridge.initGPFIFO(1, FIFO_MODE_MCP_RX, PL_SIZE_MCP_8, 8, 0b11110, TXRET_MCP_THREE, flagArray, 2);
        bridge.initFilter(0, 1, 0x2);
    }

    bridge.setContMode(CMODE_MCP_CFD_NORM);

    gpio_put(17, 0);

    uint8_t message_buff[16] = {0};
    char data[8] = "KIRKY\0";

    create_message_obj(message_buff, (uint8_t*)data, CAN_FD_BASE_MCP, PL_SIZE_MCP_8, 0x2, 0);

    int m_sent = 0;
    int m_resc = 0;
    
    if (MODE == 0) {
        sleep_ms(2000);
    }

    while (1) {
        uint8_t buff[255];
        char mbuff[255];
        if (MODE == 0) {
            printf("SENDING:  ");
            for (int i=0; i<16; i++) {
                printf("%02x ", message_buff[i]);
            }
            printf("\n");

            bridge.pushTXFIFO(1, message_buff, 16);
            bridge.reqSendTXFIFO(1);
            m_sent++;
            sprintf(mbuff, "SENT: %d", m_sent);
            display.clear();
            pico_ssd1306::drawText(&display, font_8x8, mbuff, 0, 0);
            display.sendBuffer();
            sleep_ms(250);
        }
        else if (MODE == 1) {
            if (message_waiting == true) {
                gpio_put(17, 0);
                bridge.popRXFIFO(1, buff, 16);
                m_resc++;
                printf("RECEIVED: ");
                for (int i=0; i<16; i++) {
                    printf("%02x ", buff[i]);
                }
                printf("\n");

                sprintf(mbuff, "RESC: %d", m_resc);
                display.clear();
                pico_ssd1306::drawText(&display, font_8x8, mbuff, 0, 0);
                memset(mbuff, '\0', 16);
                memcpy(mbuff, buff+8, 8);
                pico_ssd1306::drawText(&display, font_8x8, mbuff, 0, 9);
                display.sendBuffer();
                message_waiting = false;
            }
        }
    }

}