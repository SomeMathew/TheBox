// TODO explain

#ifndef _DEV_BOX_CONTROL_H
#define _DEV_BOX_CONTROL_H

#include "servo.h"
#include <avr/io.h>

#define LID_MOTOR SERVO_CHANNELA
#define LOCK_MOTOR SERVO_CHANNELB

// Positions for the box lock
#define LOCK_UNLOCKED_POSITION 0
#define LOCK_LOCKED_POSITION 90

// Positions for the box lid
#define LID_CLOSED_POSITION 180
#define LID_OPEN_POSITION 30

/**
 * Initializes the box for use by preparing its motors
 */
int box_init();

/**
 * Will unlock the box, if the box is closed
 * @return 0 if success, negative if failure
 */
int box_unlock();

/**
 * Will lock the box, if the box is closed
 * @return 0 if success, negative if failure
 */
int box_lock();

/**
 * Will close the box, if the box is open
 * @return 0 if success, negative if failure
 */
int box_close();

/**
 * Will OPEN the box, if the box is closed
 * @return 0 if success, negative if failure
 */
int box_open();

/**
 * Checks whether or not the box is open
 * @return 1 if open, 0 if not
 */
int box_isOpen();

/**
 * Checks whether or not the box is closed
 * @return 1 if closed, 0 if open
 */
int box_isClosed();

#endif