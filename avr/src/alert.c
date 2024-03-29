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


/**
 * Set the alert to ARMED.
 * 
 * This will clear any pending interrupt on the LSM303 and enables
 * the EXTINT0 interrupt.
 */
static inline void armAlert() {
	lsm303_clear_latched_interrupt();

	// Enable interrupt 0 
	EIMSK |= _BV(INT0);
	
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		alarmState = ALERT_STATE_ARMED;
	}
}

/**
 * Disables the EXTINT0 interrupt and clears the latched interrupt on the
 * LSM303.
 * 
 * This does not change the state machine. 
 */
static inline void disableAlertInterrupt() {
	// Disable interrupt 0 
	EIMSK &= ~_BV(INT0);
	lsm303_clear_latched_interrupt();
}

/**
 * Set the alert to DISARMED.
 * 
 * Clears pending interrupt on the LSM303 and disables EXTINT0 interrupt.
 */
static inline void disarmAlert() {
	disableAlertInterrupt();
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		alarmState = ALERT_STATE_DISARMED;
	}
}

/*
 * @see alert.h
 */
uint8_t alert_getstatus() {
	return alarmState;
}

/*
 * @see alert.h
 */
void alert_run(uint8_t run) {
	if ((alarmState == ALERT_STATE_DISARMED || alarmState == ALERT_STATE_OK) && run == ALERT_RUN_ARMED) {
		armAlert();
		alarmState = run;
	} else if (alarmState == ALERT_STATE_ARMED && run == ALERT_RUN_DISARM) {
		disarmAlert();
		alarmState = run;
	}
}


/*
 * @see alert.h
 */
int alert_init() {
	lsm303_init(LSM303_DATA_RATE_25HZ, LSM303_FS_4G);
	lsm303_set_interrupt(ALERT_ACCEL_THRESHOLD, ALERT_ACCEL_DURATION);
	
	// Wait for the lsm303 to stabilize otherwise we get a false interrupt
	_delay_ms(ALERT_INIT_DELAY_MS);
	
	// Int on rising edge.
	EICRA |= _BV(ISC00) | _BV(ISC01);
	
	alarmState = ALERT_STATE_OK;
	return 1;
}

/**
 * Wait using the timer0, used to quiet the alarm on interuders status.
 * 
 * This will prevent multiple sequential event of alert in a 5sec interval.
 */
static void wait() {
  TCCR0B |= (1 << CS02) | (1 << CS00); // Set the prescalar to 1024
  TIMSK0 |= (1 << TOIE0); // Enable the Timer0 overflow Interrupt
  TCNT0 = 0; // Set the counter to 0
  timerCount = 0;
}


/**
 * Waiting ISR to manage the quiet time of the alarm on intruder alert.
 * 
 * @see #wait()
 */
ISR(TIMER0_OVF_vect) {
    timerCount++;
    // B/c 16 MHz / 1024 / 256 => overflows / s * 5 s
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


/**
 * EXTINT0 when LSM303 initiates an interrupt.
 * 
 * This is the intruder alert.
 */
ISR(INT0_vect) {
	if (alarmState == ALERT_STATE_ARMED) {
		disableAlertInterrupt();
		spicmd_send(SPICMD_BBB_ALERT);
		ioctl_write(&LED_ALIVE_PORT, LED_ALIVE_IO, 1);
		alarmState = ALERT_STATE_INTRUDER;
		wait();
	}
}
