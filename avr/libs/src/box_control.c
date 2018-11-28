#include "box_control.h"
#include "util/delay.h"
#include "pin_config.h"
#include "ioctl.h"

/**
 * Initializes the box for use by preparing its motors
 */
int box_init() {
    int success = 0;

    //Initialize the reed switch
    ioctl_setdir(&BOX_SWITCH_DDR, BOX_SWITCH_IO, INPUT);
    ioctl_pullup(&BOX_SWITCH_PORT, BOX_SWITCH_IO);

    // Initialize the servo motors

    success -= servo_init();
    success -= servo_channel_init_angle(LID_MOTOR, LID_CLOSED_POSITION);
    while(box_isOpen()) {
        // Wait
    };
    success -= servo_channel_init_angle(LOCK_MOTOR, LOCK_LOCKED_POSITION);

    return success;
}

/**
 * Will unlock the box, if the box is closed
 * @return 0 if success, negative if failure
 */
int box_unlock() {
    if(box_isOpen()) {
        return -1;
    }

    return servo_write(LOCK_MOTOR, LOCK_UNLOCKED_POSITION);
}

/**
 * Will lock the box, if the box is closed
 * @return 0 if success, negative if failure
 */
int box_lock() {
    if(box_isOpen()) {
        return -1;
    }

    return servo_write(LOCK_MOTOR, LOCK_LOCKED_POSITION);
}

/**
 * Will close the box, if the box is open
 * @return 0 if success, negative if failure
 */
int box_close() {
    if(box_isClosed()) {
        return -1;
    }

    return servo_write(LID_MOTOR, LID_CLOSED_POSITION);
}

/**
 * Will open the box, if the box is closed
 * @return 0 if success, negative if failure
 */
int box_open() {
    if(box_isOpen()) {
        return -1;
    }

    return servo_write(LID_MOTOR, LID_OPEN_POSITION);
}

/**
 * Checks whether or not the box is open
 * @return 1 if open, 0 if not
 */
int box_isOpen() {
    return ioctl_read(&BOX_SWITCH_PIN, BOX_SWITCH_IO);
}

/**
 * Checks whether or not the box is closed
 * @return 1 if closed, 0 if open
 */
int box_isClosed() {
    if(box_isOpen()) {
        return 0;
    } else {
        return 1;
    }
}