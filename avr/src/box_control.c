#include "box_control.h"
#include "util/delay.h"
#include "pin_config.h"
#include "spi_command.h"
#include "ioctl.h"
// TODO remove #include "uart.h" 
// TODO remove #include <stdlib.h> 

typedef uint8_t State;
static volatile State state; 

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

    state = BOX_STATE_IDLE_CLOSED;
    return success;
}

/**
 * Returns the state that the box is in
 * 
 * BOX_STATE_IDLE_OPEN     0x01
 * BOX_STATE_PENDING_OPEN  0x02
 * BOX_STATE_IDLE_CLOSED   0x03
 * BOX_STATE_PENDING_CLOSE 0x04
 */
int box_getState() {
    return state;
}

/**
 * Sets the state that the box is in
 * 
 * BOX_STATE_IDLE_OPEN     0x01
 * BOX_STATE_PENDING_OPEN  0x02
 * BOX_STATE_IDLE_CLOSED   0x03
 * BOX_STATE_PENDING_CLOSE 0x04
 */
void box_setState(uint8_t newState) {
    state = newState;
}

/**
 * Tells the box to take care of the state that it is currently in.
 */ 
void box_handleCurrentState() {
    int isOpen;
    switch(state) {
        case BOX_STATE_PENDING_OPEN:
            box_unlock();
            box_open();
            break;
        case BOX_STATE_PENDING_CLOSE:
            box_close();
            while(box_isOpen()){};
            box_lock();
            break;
        case BOX_STATUS_QUERY:
            isOpen = box_isOpen();
            if(isOpen) {
                spicmd_send(SPICMD_RESP_OPENED);
                state = BOX_STATE_IDLE_OPEN;
            } else {
                spicmd_send(SPICMD_RESP_CLOSED);
                state = BOX_STATE_IDLE_CLOSED;
            }
           	// TODO remove fprintf(&uartStream, "Is switch open? %d\n", box_isOpen());

    }
}

/**
 * Will close and lock the box
 */
void cmd_closeBox() {

}

/**
 * Returns whether or not the box is closed
 * @return 0 if closed, 1 if open
 */
int cmd_getStatus() {
    return box_isOpen();
}

/**
 * Will unlock the box, if the box is closed
 * @return 0 if success, negative if failure
 */
int box_unlock() {
    if(box_isOpen()) {
        return -1;
    }

    int success = 0;
    for(int angle = LOCK_LOCKED_POSITION; angle <= LOCK_UNLOCKED_POSITION; angle += 30) {
        success -= servo_write(LOCK_MOTOR, angle);
        _delay_ms(105); // 0.21 s / 60 deg * 30 deg
    }
    return success;
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

    state = BOX_STATE_IDLE_CLOSED;
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

    state = BOX_STATE_IDLE_OPEN;
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