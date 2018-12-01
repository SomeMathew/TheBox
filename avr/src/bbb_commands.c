#include "bbb_commands.h"
#include "box_control.h"

/**
 * Will unlock and open the box
 */
int spicmd_callback_unlockopen() {
    box_setState(BOX_STATE_PENDING_OPEN);
    return 0;
}

/**
 * Will close and lock the box
 */
int spicmd_callback_closelock() {
    box_setState(BOX_STATE_PENDING_CLOSE);
    return 0;
}

/**
 * Returns whether or not the box is closed
 * @return 0 if closed, 1 if open
 */
int spicmd_callback_checkstatus() {
    box_setState(BOX_STATUS_QUERY);
    return 0;
};
