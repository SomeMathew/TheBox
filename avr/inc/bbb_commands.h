/**
 * This file provides commands that can be called directly from the SPI interface
 * to operate the box
 */

#ifndef _DEV_BBB_COMMANDS_H
#define _DEV_BBB_COMMANDS_H

#include <avr/io.h>
#include "box_control.h"

/**
 * Will unlock and open the box
 */
int spicmd_callback_unlockopen();

/**
 * Will close and lock the box
 */
int spicmd_callback_closelock();

/**
 * Returns whether or not the box is closed
 * @return 0 if closed, 1 if open
 */
int spicmd_callback_checkstatus();

#endif