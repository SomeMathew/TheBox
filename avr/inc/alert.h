#ifndef _DEV_ALERT_H
#define _DEV_ALERT_H

#include "pin_config.h"

#define ALERT_INIT_DELAY_MS			(1500)

#define ALERT_RUN_ARMED				(10)
#define ALERT_RUN_DISARM			(11)

#define ALERT_ACCEL_THRESHOLD		(5)
#define ALERT_ACCEL_DURATION 		(2)

void alert_run(uint8_t run);
uint8_t alert_getstatus();
int alert_init();
int alert_execute();

#endif /* _DEV_ALERT_H */
