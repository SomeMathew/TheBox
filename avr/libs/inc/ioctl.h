/**
 * Convenience inline function module to control the GPIO on the AVR.
 * 
 * Note: When using the register, &REG_NAME must be used instead of 
 * REG_NAME like with normal direct registers manipulations such as
 * REG_NAME |= _BV(2).
 * 
 * Example:
 * 		ioctl_setdir(&DDRB, PB4, OUTPUT);
 */

#ifndef _DEV_IOCTL_H
#define _DEV_IOCTL_H

#include <stdbool.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>

#define OUTPUT (1)
#define INPUT (0)

/**
 * Sets the direction on a GPIO.
 * 
 * @param ddrReg The mem-mapped register address for DDRx.
 * @param io The pin number.
 * @param dir  OUTPUT|INPUT
 */
static inline void ioctl_setdir(volatile uint8_t * ddrReg, uint8_t io, uint8_t dir) {
	switch (dir) {
		case INPUT:
			*ddrReg &= ~(_BV(io));
			break;
		case OUTPUT:
			*ddrReg |= _BV(io);
			break;
	}
}

/**
 * Sets the pullup on a GPIO.
 * 
 * @param portReg The mem-mapped register address for PORTx.
 * @param io The pin number (ie. PBx)
 */
static inline void ioctl_pullup(volatile uint8_t * portReg, uint8_t io) {
	*portReg |= _BV(io);
}

/**
 * Reads a GPIO.
 * 
 * @param pinReg The mem-mapped register address for PINx.
 * @param io The pin number (ie. PBx)
 * @returns true for High, false for Low
 */
static inline bool ioctl_read(volatile uint8_t * pinReg, uint8_t io) {
	return *pinReg & _BV(io);
}

/**
 * Set a GPIO to high/low.
 * 
 * @param portReg The mem-mapped register address for PORTx.
 * @param io The pin number (ie. PBx)
 * @param val Value to write as boolean.
 */
static inline void ioctl_write(volatile uint8_t * portReg, uint8_t io, bool val) {
	if (val) {
		*portReg |= _BV(io);
	} else {
		*portReg &= ~(_BV(io));
	}
}

/**
 * Toggles a GPIO.
 * 
 * @param portReg The mem-mapped register address for PORTx.
 * @param io The pin number (ie. PBx)
 */
static inline void ioctl_toggle(volatile uint8_t * portReg, uint8_t io) {
	*portReg ^= _BV(io);
} 

/**
 * Set a GPIO as tri-stated.
 * 
 * @param ddrReg The mem-mapped register address for DDRx.
 * @param portReg The mem-mapped register address for PORTx.
 * @param io The pin number (ie. PBx)
 */
static inline void ioctl_tristate(volatile uint8_t * ddrReg, volatile uint8_t * portReg, uint8_t io) {
	*portReg &= ~(_BV(io));
	*ddrReg &= ~(_BV(io));
}

#endif /* _DEV_IOCTL_H */
