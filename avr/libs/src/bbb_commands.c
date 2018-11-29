#include "bbb_commands.h"
#include "box_control.h"

/**
 * Will unlock and open the box
 */
void cmd_openBox() {
    box_unlock();
    box_open();
}

/**
 * Will close and lock the box
 */
void cmd_closeBox() {
    box_close();
    while(box_isOpen()){};
    box_lock();
}

/**
 * Returns whether or not the box is closed
 * @return 0 if closed, 1 if open
 */
int cmd_getStatus() {
    return box_isOpen();
};
