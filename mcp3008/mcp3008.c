/**************************************************************************************************
 * mcp3008.c
 *
 * Description
 * ===========
 *  Library for MCP3008 ADC on Raspberry Pi v1.0 model B
 *
 * Details
 * =======
 *  Author: Jelle De Vleeschouwer
 *  Copyright (C) 2017 Jelle De Vleeschouwer
 *
 **************************************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <bcm2835.h>

#include "mcp3008.h"

//#define DEBUG_MCP3008
#ifdef DBG_MCP3008
    #define dbg printf
#else
    #define dbg(...)
#endif

/**************************************************************************************************
 * Constants
 **************************************************************************************************/

#define RASPBERRY_PI_1_MAX_GPIO     (26)

/**************************************************************************************************
 *  Type Definitions
 **************************************************************************************************/

const int mcp3008_available_gpio_v1b[RASPBERRY_PI_1_MAX_GPIO] = {
    0,              0,
    RPI_GPIO_P1_03, 0,
    RPI_GPIO_P1_05, 0,
    RPI_GPIO_P1_07, RPI_GPIO_P1_08,
    0,              RPI_GPIO_P1_10,
    RPI_GPIO_P1_11, RPI_GPIO_P1_12,
    RPI_GPIO_P1_13, 0,
    RPI_GPIO_P1_15, RPI_GPIO_P1_16,
    0,              RPI_GPIO_P1_18,
    0,              0,
    0,              RPI_GPIO_P1_22,
    0,              RPI_GPIO_P1_24,
    0,              RPI_GPIO_P1_26
};

static int mcp3008_chip_select = RPI_GPIO_P1_16;

/**************************************************************************************************
    BCM2835
 **************************************************************************************************/

/**
 *  Initialises BCM2835 on the RPi for SPI-communication
 */
static int
bcm2835_spi_start(void)
{
    if (!bcm2835_init()) {
        fprintf(stderr, "bcm2835_init failed. Are you running as root??\n");
        return -1;
    }

    if (!bcm2835_spi_begin()) {
        fprintf(stderr, "bcm2835_spi_begin failed. Are you running as root??\n");
        return -1;
    }

    return 0;
}

/**
 *  Stops and free all required resources for SPI-communication on the RPi
 */
static void
bcm2835_spi_stop(void)
{
    bcm2835_spi_end();
    bcm2835_close();
}

/**************************************************************************************************
    MCP3008
 **************************************************************************************************/

/**
 *  Pull Chip Select for AD-converter low
 */
static void
mcp3008_select(void)
{
    bcm2835_gpio_write(mcp3008_chip_select, LOW);
}

/**
 *  Make Chip Select for AD-convert high
 */
static void
mcp3008_deselect(void)
{
    bcm2835_gpio_write(mcp3008_chip_select, HIGH);
}

/**
 *  Validate user passed chip select pin or use default when none is given.
 */
static int
valid_chip_select(int _chip_select)
{
    _chip_select--;

    if (_chip_select == -1) {
        mcp3008_chip_select = RPI_GPIO_P1_16;
        return 1; // Use default Chip Select
    } else if (_chip_select < -1 || _chip_select > RASPBERRY_PI_1_MAX_GPIO) {
        return 0; // OOB check, invalid if OOB
    } else {
        if (mcp3008_available_gpio_v1b[_chip_select]) {
            mcp3008_chip_select = mcp3008_available_gpio_v1b[_chip_select];
            return 1;
        } else {
            return 0;
        }
    }
}

/**
 *  Initalises the SPI-interface (mode: 0,0) with chip select given by 'chip_select'. If '0'
 *  is passed the default chip select pin GPIO23 (16) will be assigned.
 */
int mcp3008_init(int _chip_select)
{
    // Initialise SPI-module on Raspberry Pi
    bcm2835_spi_start();

    // Configure CS pin
    if (!valid_chip_select(_chip_select)) {
        fprintf(stderr, "Invalid chip_select pin: %d\n, only GPIO (non-SPI) pins are valid\n", _chip_select);
        return -1;
    }

    // Configure SPI-interface
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536);
    bcm2835_spi_chipSelect(BCM2835_SPI_CS_NONE);

    // Make a GPIO pin the chip select for the MCP3008
    bcm2835_gpio_fsel(mcp3008_chip_select, BCM2835_GPIO_FSEL_OUTP);
    mcp3008_deselect();

    return 0;
}

/**
 *  Stops and free all required resources for SPI-communication on the RPi
 */
void mcp3008_stop(void)
{
    bcm2835_spi_stop();
}

/**
 *  Print transmitted and received bytes
 */
static void mcp3008_dbg(uint8_t tx, uint8_t rx)
{
    dbg("TX: 0x%02X RX: 0x%02X\n", tx, rx);
}

/**
 *  Read a single conversion from a certain channel on the MCP3008
 */
uint16_t mcp3008_readChannel(enum mcp3008_channel channel, int single_ended)
{
    uint8_t command = 0x01;
    uint16_t data = 0;
    uint8_t ret = 0;

    // Pull down chip select line
    mcp3008_select();

    // Transmit first 8-bits with a START-bit as LSB
    ret = bcm2835_spi_transfer(command);
    mcp3008_dbg(command, ret);

    // Transmit command for specific and receive first 2 MSBs
    command = (single_ended ? 0x01 : 0x00) << 7;
    command = command | ((uint8_t)channel) << 4;
    ret = bcm2835_spi_transfer(command);
    mcp3008_dbg(command, ret);
    data = (uint16_t)ret << 8;

    // Transmit dummy-byte for 8 LSBs
    ret = bcm2835_spi_transfer(0x00);
    data = data | (uint16_t)ret;
    mcp3008_dbg(0x00, ret);

    // Make chip select idle again
    mcp3008_deselect();

    return data;
}

/**************************************************************************************************
 *  mcp3008.c
 **************************************************************************************************/
