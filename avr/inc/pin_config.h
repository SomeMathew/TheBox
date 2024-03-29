/**
 * Global definitions of the GPIO used in this project.
 */

#ifndef _DEV_IO_CONFIG_H
#define _DEV_IO_CONFIG_H

#define SERVO_A_DDR 	DDRB
#define SERVO_A_IO 		PB1

#define SERVO_B_DDR 	DDRC
#define SERVO_B_PORT	PORTC
#define SERVO_B_PIN		PINC
#define SERVO_B_IO 		PC0

#define LED_ALIVE_DDR	DDRD
#define LED_ALIVE_PORT	PORTD
#define LED_ALIVE_PIN	PIND
#define LED_ALIVE_IO	PD4

#define BBB_STATUS_DDR	DDRD
#define BBB_STATUS_PORT	PORTD
#define BBB_STATUS_PIN	PIND
#define BBB_STATUS_IO	PD5

#define BOX_SWITCH_DDR	DDRC
#define BOX_SWITCH_PORT	PORTC
#define BOX_SWITCH_PIN	PINC
#define BOX_SWITCH_IO	PC3

#define ACCEL_INT_DDR	DDRD
#define ACCEL_INT_PORT	PORTD
#define ACCEL_INT_PIN	PIND
#define ACCEL_INT_IO	PD2

#define ALERT_DDR 		DDRD
#define ALERT_PORT 		PORTD
#define ALERT_PIN		PIND
#define ALERT_IO 		PD2

#endif /* _DEV_IO_CONFIG_H */
