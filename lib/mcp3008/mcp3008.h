/**************************************************************************************************
 * mcp3008.h
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

#include <inttypes.h>

#ifndef __MCP3008_H_
#define __MCP3008_H_

/*
 * Required pin-assignment for MCP3008 interface (Raspberry Pi v1.0b)
 *
 *              P1___________________________
 *             _|                            |
 *          - |_| (1)  3V3           5V  (2) | -
 *          -   | (3)  GPIO2         5V  (4) | -
 *          -   | (5)  GPIO3        GND  (6) | -
 *          -   | (7)  GPIO4     GPIO14  (8) | -
 *          -   | (9)  GND       GPIO15 (10) | -
 *          -   | (11) GPIO17    GPIO18 (12) | -
 *          -   | (13) GPIO27       GND (14) | -
 *          -   | (15) GPIO22    GPIO23 (16) | - CHIP_SELECT (default)
 *          -   | (17) 3V3       GPIO24 (18) | -
 *     MOSI -   | (19) GPIO10       GND (20) | -
 *     MISO -   | (21) GPIO9     GPIO25 (22) | -
 *     SCLK -   | (23) GPIO11     GPIO8 (24) | -
 *          -   | (25) GND        GPIO7 (26) | -
 *              |____________________________|
 */

#define MCP3008_SE          (1)

enum mcp3008_channel
{
    MCP3008_CH0 = 0,
    MCP3008_CH1,
    MCP3008_CH2,
    MCP3008_CH3,
    MCP3008_CH4,
    MCP3008_CH5,
    MCP3008_CH6,
    MCP3008_CH7
};


/**
 *  Initalises the SPI-interface (mode: 0,0) with chip select given by 'chip_select'. If '0'
 *  is passed the default chip select pin GPIO23 (16) will be assigned.
 */
int mcp3008_init(int chip_select);

/**
 *  Stops and free all required resources for SPI-communication on the RPi
 */
void mcp3008_stop(void);

/**
 *  Read a single conversion from a certain channel on the MCP3008
 */
uint16_t mcp3008_readChannel(enum mcp3008_channel channel, int single_ended);

#endif /* __MCP3008_H_ */
