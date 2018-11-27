#include "box_control.h"

/**
 * Initializes the box for use by preparing its motors
 */
int box_init() {
    int success = 0;

    // Initialize the servo motors
    /* TODO - may want to add check to see if open or close, and adjust motors accordingly
            - for now, I'll leave with assuming the box is closed and locked beforehand */
    success -= servo_init();
    // TODO once I have lock in place success -= servo_channel_init_angle(LOCK_MOTOR, LOCK_LOCKED_POSITION);
    success -= servo_channel_init_angle(LID_MOTOR, LID_CLOSED_POSITION);

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
    //TODO implement
    return 0;
}

/**
 * Checks whether or not the box is closed
 * @return 1 if closed, 0 if open
 */
int box_isClosed() {
    // TODO implement
    return 0;
}