#ifndef _DEV_IOCTL_H
#define _DEV_IOCTL_H

#include <stdbool.h>
#include <avr/io.h>
#include <avr/sfr_defs.h>

#define OUTPUT (1)
#define INPUT (0)

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

static inline void ioctl_pullup(volatile uint8_t * portReg, uint8_t io) {
	*portReg |= _BV(io);
}

static inline bool ioctl_read(volatile uint8_t * pinReg, uint8_t io) {
	return *pinReg & _BV(io);
}

static inline void ioctl_write(volatile uint8_t * portReg, uint8_t io, bool val) {
	if (val) {
		*portReg |= _BV(io);
	} else {
		*portReg &= ~(_BV(io));
	}
}

static inline void ioctl_toggle(volatile uint8_t * portReg, uint8_t io) {
	*portReg ^= _BV(io);
} 

static inline void ioctl_tristate(volatile uint8_t * ddrReg, volatile uint8_t * portReg, uint8_t io) {
	*portReg &= ~(_BV(io));
	*ddrReg &= ~(_BV(io));
}

#endif /* _DEV_IOCTL_H */
