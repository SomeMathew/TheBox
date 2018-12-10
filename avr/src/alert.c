// Alert module for the lsm303 with interrupt enabled

#include <stdint.h>
#include <stddef.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "spi_command.h"
#include "alert.h"
#include "lsm303.h"
#include "pin_config.h"
#include "ioctl.h"

#define ALERT_STATE_OFF			(0)
#define ALERT_STATE_OK			(1)
#define ALERT_STATE_ARMED 		(ALERT_RUN_ARMED)
#define ALERT_STATE_INTRUDER 	(3)
#define ALERT_STATE_DISARMED	(ALERT_RUN_DISARM)

static void wait();

typedef uint8_t State;
volatile State alarmState = ALERT_STATE_OFF;

volatile int timerCount = 0;


static inline void arm_alert() {
	lsm303_clear_latched_interrupt();

	// Enable interrupt 0 
	EIMSK |= _BV(INT0);
	
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		alarmState = ALERT_STATE_ARMED;
	}
}

static inline void disableAlertInterrupt() {
	// Disable interrupt 0 
	EIMSK &= ~_BV(INT0);
	lsm303_clear_latched_interrupt();
}

static inline void disarm_alert() {
	disableAlertInterrupt();
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		alarmState = ALERT_STATE_DISARMED;
	}
}

uint8_t alert_getstatus() {
	return alarmState;
}

void alert_run(uint8_t run) {
	if ((alarmState == ALERT_STATE_DISARMED || alarmState == ALERT_STATE_OK) && run == ALERT_RUN_ARMED) {
		arm_alert();
		alarmState = run;
	} else if (alarmState == ALERT_STATE_ARMED && run == ALERT_RUN_DISARM) {
		disarm_alert();
		alarmState = run;
	}
}



int alert_init() {
	lsm303_init(LSM303_DATA_RATE_25HZ, LSM303_FS_4G);
	lsm303_set_interrupt(ALERT_ACCEL_THRESHOLD, ALERT_ACCEL_DURATION);
	
	// Wait for the lsm303 to stabilize otherwise we get a false interrupt
	_delay_ms(ALERT_INIT_DELAY_MS);
	
	// EICRA already at 00 for active low interrupt on INT0
	EICRA |= _BV(ISC00) | _BV(ISC01);
	
	alarmState = ALERT_STATE_OK;
	return 1;
}

static void wait() {
  TCCR0B |= (1 << CS02) | (1 << CS00); // Set the prescalar to 1024
  TIMSK0 |= (1 << TOIE0); // Enable the Timer0 overflow Interrupt
  TCNT0 = 0; // Set the counter to 0
  timerCount = 0;
}


// Interrupt vector to handle the overflow of Timer0
ISR(TIMER0_OVF_vect) {
    timerCount++;
    // B/c 16 MHz / 1024 / 256 => overflows / s * 30 s
    if(timerCount >= 305) {
      if (alarmState == ALERT_STATE_INTRUDER) {
		  ioctl_write(&LED_ALIVE_PORT, LED_ALIVE_IO, 0);
		  alarmState = ALERT_STATE_OK;
	  }
      timerCount = 0;
      // Disable timer
      TCCR0B = 0;
      // disable timer interrupt
      TIMSK0 &= ~(_BV(TOIE0));
      
    }
}



ISR(INT0_vect) {
	if (alarmState == ALERT_STATE_ARMED) {
		disableAlertInterrupt();
		spicmd_send(SPICMD_BBB_ALERT);
		ioctl_write(&LED_ALIVE_PORT, LED_ALIVE_IO, 1);
		alarmState = ALERT_STATE_INTRUDER;
		wait();
	}
}
