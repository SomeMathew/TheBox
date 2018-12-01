#include <stdint.h>
#include <stddef.h>
#include <avr/sfr_defs.h>
#include "lsm303.h"
#include "pin_config.h"
#include "i2c.h"
#include "ioctl.h"


/****************************************************/
// Define of the Register addresse from Adafruit LSM303DLHC Arduino library
#define	LSM303_REGISTER_ACCEL_CTRL_REG1_A       (0x20) 
#define	LSM303_REGISTER_ACCEL_CTRL_REG2_A       (0x21) 
#define	LSM303_REGISTER_ACCEL_CTRL_REG3_A       (0x22) 
#define	LSM303_REGISTER_ACCEL_CTRL_REG4_A       (0x23) 
#define	LSM303_REGISTER_ACCEL_CTRL_REG5_A       (0x24) 
#define	LSM303_REGISTER_ACCEL_CTRL_REG6_A       (0x25) 
#define	LSM303_REGISTER_ACCEL_REFERENCE_A       (0x26) 
#define	LSM303_REGISTER_ACCEL_STATUS_REG_A      (0x27) 
#define	LSM303_REGISTER_ACCEL_OUT_X_L_A         (0x28)
#define	LSM303_REGISTER_ACCEL_OUT_X_H_A         (0x29)
#define	LSM303_REGISTER_ACCEL_OUT_Y_L_A         (0x2A)
#define	LSM303_REGISTER_ACCEL_OUT_Y_H_A         (0x2B)
#define	LSM303_REGISTER_ACCEL_OUT_Z_L_A         (0x2C)
#define	LSM303_REGISTER_ACCEL_OUT_Z_H_A         (0x2D)
#define	LSM303_REGISTER_ACCEL_FIFO_CTRL_REG_A   (0x2E)
#define	LSM303_REGISTER_ACCEL_FIFO_SRC_REG_A    (0x2F)
#define	LSM303_REGISTER_ACCEL_INT1_CFG_A        (0x30)
#define	LSM303_REGISTER_ACCEL_INT1_SOURCE_A     (0x31)
#define	LSM303_REGISTER_ACCEL_INT1_THS_A        (0x32)
#define	LSM303_REGISTER_ACCEL_INT1_DURATION_A   (0x33)
#define	LSM303_REGISTER_ACCEL_INT2_CFG_A        (0x34)
#define	LSM303_REGISTER_ACCEL_INT2_SOURCE_A     (0x35)
#define	LSM303_REGISTER_ACCEL_INT2_THS_A        (0x36)
#define	LSM303_REGISTER_ACCEL_INT2_DURATION_A   (0x37)
#define	LSM303_REGISTER_ACCEL_CLICK_CFG_A       (0x38)
#define	LSM303_REGISTER_ACCEL_CLICK_SRC_A       (0x39)
#define	LSM303_REGISTER_ACCEL_CLICK_THS_A       (0x3A)
#define	LSM303_REGISTER_ACCEL_TIME_LIMIT_A      (0x3B)
#define	LSM303_REGISTER_ACCEL_TIME_LATENCY_A    (0x3C)
#define	LSM303_REGISTER_ACCEL_TIME_WINDOW_A     (0x3D)
/****************************************************/

#define LSM303_REGISTER_AUTO_INC (0x80)

#define LSM303DLHC_ADDRESS_LIN_ACCEL (0b00110010)

// CTRL_REG1_A (20h)
#define ODR		(4)
#define	LPEN	(3)
#define ZEN		(2)
#define YEN		(1)
#define	XEN		(0)

// CTRL_REG4_A (23h)
#define BDU 	(7)
#define BLE		(6)
#define FS		(4)
#define HR		(3)
#define SIM		(0)

// STATUS_REG_A (27h)
#define ZYXOR	(7)
#define ZOR		(6)
#define YOR		(5)
#define	XOR		(4)
#define ZYXDA	(3)
#define ZDA		(2)
#define YDA		(1)
#define XDA		(0)

#define ACCEL_READING_SIZE 7


static void decodeReading(uint8_t * rawReading, struct lsm303_accel_reading * reading);

/**
 * @see lsm303.h
 */
int lsm303_init(enum lsm303_data_rate rate, enum lsm303_full_scale scale) {
	uint8_t ctrlRegValue;
	// Set ACCEL_CTRL_REG1_A: Output Data Rate and Enable all axis
	ctrlRegValue = (rate << (ODR)) | _BV(ZEN) | _BV(YEN) | _BV(XEN);
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, &ctrlRegValue, 1);

	
	// TODO enable interrupt in CTRL_REG3_A
	
	// Set ACCEL_CTRL_REG4_A: Full-scale selection, 
	ctrlRegValue = (scale << (FS));
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG4_A, &ctrlRegValue, 1);
	
	return 1;
}

/**
 * @see lsm303.h
 */
int lsm303_read(struct lsm303_accel_reading * reading) {
	uint8_t rawReading[ACCEL_READING_SIZE];
	
	// Read status + all 6 registers in auto-increment mode for the raw register values
	i2c_master_read(LSM303DLHC_ADDRESS_LIN_ACCEL, 
					(LSM303_REGISTER_ACCEL_STATUS_REG_A | LSM303_REGISTER_AUTO_INC), 
					rawReading, ACCEL_READING_SIZE);

	reading->rawStatus = rawReading[0];
	
	//~ fprintf(&uartStream, "Raw:, val: %"PRIx8", xl: %"PRIx8", xh: %"PRIx8", yl: %"PRIx8", yh: %"PRIx8", zl: %"PRIx8", zh:%"PRIx8"\n", rawReading[0],rawReading[1],rawReading[2],rawReading[3],rawReading[4],rawReading[5], rawReading[6]);
	// Check if the data is valid
	if (rawReading[0] & _BV(ZYXDA)) {
		decodeReading(rawReading, reading); 
	} else {
		reading->status = LSM303_DATA_NREADY;
	}
	
	return 1;
}

/**
 * Decodes the accelerometer readings for the x,y,z axis.
 * 
 * The raw data given by the lsm303 are 12 bit 2's complement 
 * left-aligned numbers read from a 2 8bit registers as 16 bit output. 
 * This is not well documented in the datasheet.
 * 
 * The code found in this section was adapted from my previous project
 * that used this accelerometer. It is found on github under a
 * SpaceConcordia Project.
 * 
 * @param rawReading 	Reading from the device of the status + 3-axis
 * @param reading		Pointer to a struct as the output.
 */ 
static void decodeReading(uint8_t * rawReading,  struct lsm303_accel_reading * reading) {
	// div by 16 since it is a left-aligned 12 bit number (undocumented) (safe >> 4 signed)
	reading->x = (int16_t) (((uint16_t) rawReading[2] << 8) | (rawReading[1]))/16;
	reading->y = (int16_t) (((uint16_t) rawReading[4] << 8) | (rawReading[3]))/16;
	reading->z = (int16_t) (((uint16_t) rawReading[6] << 8) | (rawReading[5]))/16;
	
	reading->status = LSM303_OK;
	return;
}
