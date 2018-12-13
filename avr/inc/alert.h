/**
 * Module to control an alert from a EXTINT 0 ISR connected to the
 * lsm303.h acceleremoter.
 * 
 * This module will respond to a rising edge on the on external pin 0 when
 * it is ARMED. 
 * 
 * This module is directly tied to the lsm303.h driver to control the
 * LSM303LDHC interrupt and status clear.
 */

#ifndef _DEV_ALERT_H
#define _DEV_ALERT_H

#include "pin_config.h"

#define ALERT_INIT_DELAY_MS			(1500)

#define ALERT_RUN_ARMED				(10)
#define ALERT_RUN_DISARM			(11)

/**
 * Sets the LSM303 Threshold for High interrupt event.
 */
#define ALERT_ACCEL_THRESHOLD		(4)

/**
 * Sets the LSM303 Duration of a High event for interrupt.
 */
#define ALERT_ACCEL_DURATION 		(2)

/**
 * Set the alert as enabled or disabled.
 * 
 * This can be called either once, on event or at each work loop iteration.
 * 
 * @param run ALERT_RUN_ARMED/ALERT_RUN_DISARM
 */
void alert_run(uint8_t runStatus);

/**
 * Retrieves the current state of the alert.
 * 
 * @returns ALERT_STATE_*, see .c, this is for debugging purpose.
 */ 
uint8_t alert_getstatus();

/**
 * Initializes the alert module and the LSM303 driver.
 * 
 * <p>
 * Note: This will not ARM the alert.
 */
int alert_init();

#endif /* _DEV_ALERT_H */
