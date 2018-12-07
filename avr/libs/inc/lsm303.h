#ifndef _DEV_LSM303_H
#define _DEV_LSM303_H

#include <avr/io.h>
#include <avr/sfr_defs.h>

enum lsm303_data_rate {
	LSM303_DATA_RATE_1HZ =		0x1,
	LSM303_DATA_RATE_10HZ =		0x2,
	LSM303_DATA_RATE_25HZ = 	0x3,
	LSM303_DATA_RATE_50HZ = 	0x4,
	LSM303_DATA_RATE_100HZ = 	0x5,
	LSM303_DATA_RATE_200HZ = 	0x6,
	LSM303_DATA_RATE_400HZ = 	0x7
};

enum lsm303_full_scale {
	LSM303_FS_2G = 	0x0,
	LSM303_FS_4G = 	0x1,
	LSM303_FS_8G = 	0x2,
	LSM303_FS_16G =	0x3
};

enum lsm303_status {
	LSM303_OK = 0x0,
	LSM303_DATA_NREADY = 0x1
};


struct lsm303_accel_reading {
	uint8_t rawStatus;
	int16_t x;
	int16_t y;
	int16_t z;
	enum lsm303_status status;
};

/**
 * Intializes the LSM303 accelerometer device with the given data rate 
 * and reading scale.
 * 
 * The I2C peripheral driver must have been initialized by the user 
 * before this is called or an error will be produced when calling the
 * I2C driver.
 * 
 * @param rate 		Accelerometer data rate
 * @param scale		FullScale of the accelerometer
 * 
 * @return >= 0 on success, negative otherwise.
 */
int lsm303_init(enum lsm303_data_rate rate, enum lsm303_full_scale scale);

int lsm303_set_interrupt(void);

int lsm303_clear_latched_interrupt(void);

/**
 * Reads the accelerometer from the LSM303 device.
 * 
 * The reading is a full scale 16-bit value in Gs with the given 
 * initialization FS. This library does not execute the range constrain
 * to return the acceleration as a float in G.
 * 
 * @param pointer to an lsm303_accel_reading  structure where the reading
 * 			will be written by the driver.
 */
int lsm303_read(struct lsm303_accel_reading * reading);

#endif /* _DEV_LSM303_H */
