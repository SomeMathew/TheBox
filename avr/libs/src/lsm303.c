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

// CTRL_REG1_A (20h)
#define LSM303_ODR		(4)
#define	LSM303_LPEN		(3)
#define LSM303_ZEN		(2)
#define LSM303_YEN		(1)
#define	LSM303_XEN		(0)

// CTRL_REG2_A (21h)
#define LSM303_L1_CLICK		(7)
#define LSM303_L1_AOI1		(6)
#define LSM303_L1_AOI2		(5)
#define	LSM303_L1_DRDY1		(4)
#define LSM303_L1_DRDY2		(3)
#define LSM303_L1_WTM		(2)
#define	LSM303_L1_OVERRUN	(1)

// CTRL_REG3_A (22h)
#define LSM303_HPM		(6)
#define LSM303_HPCF		(4)
#define	LSM303_FDS		(3)
#define LSM303_HPCLICK	(2)
#define LSM303_HPIS2	(1)
#define	LSM303_HPIS1	(0)

// CTRL_REG4_A (23h)
#define LSM303_BDU 		(7)
#define LSM303_BLE		(6)
#define LSM303_FS		(4)
#define LSM303_HR		(3)
#define LSM303_SIM		(0)

// CTRL_REG5_A (24h)
#define LSM303_BOOT		(7)
#define LSM303_FIFO_EN	(6)
#define	LSM303_LIR_INT1	(3)
#define LSM303_D4D_INT1	(2)
#define LSM303_LIR_INT2	(1)
#define	LSM303_D4D_INT2	(0)

// CTRL_REG6_A (25h)
#define LSM303_I2_CLICK_EN	(7)
#define LSM303_I2_INT1		(6)
#define LSM303_I2_INT2		(5)
#define LSM303_BOOT_I1		(4)
#define LSM303_P2_ACT		(3)
#define LSM303_H_LACTIVE	(1)

// STATUS_REG_A (27h)
#define LSM303_ZYXOR	(7)
#define LSM303_ZOR		(6)
#define LSM303_YOR		(5)
#define	LSM303_XOR		(4)
#define LSM303_ZYXDA	(3)
#define LSM303_ZDA		(2)
#define LSM303_YDA		(1)
#define LSM303_XDA		(0)

// INT1_CFG_A (30h)
#define LSM303_AOI				(7)
#define LSM303_R6D				(6)
#define LSM303_ZHIE_ZUPE		(5)
#define LSM303_ZLIE_ZDOWNE		(4)
#define	LSM303_YHIE_YUPE		(3)
#define LSM303_YLIE_YDOWNE		(2)
#define	LSM303_XHIE_XUPE		(1)
#define LSM303_XLIE_XDOWNE		(0)

// INT1_SRC_A (31h)
#define LSM303_IA		(6)
#define LSM303_ZH		(5)
#define LSM303_ZL		(4)
#define LSM303_YH		(3)
#define LSM303_YL		(2)
#define LSM303_XH		(1)
#define LSM303_XL		(0)

#define LSM303_REGISTER_AUTO_INC (0x80)

#define LSM303DLHC_ADDRESS_LIN_ACCEL (0b00110010)

#define ACCEL_READING_SIZE 7



static void decodeReading(uint8_t * rawReading, struct lsm303_accel_reading * reading);

/*
 * @see lsm303.h
 */
int lsm303_init(enum lsm303_data_rate rate, enum lsm303_full_scale scale) {
	uint8_t ctrlRegValue;
	// Set ACCEL_CTRL_REG1_A: Output Data Rate and Enable all axis
	ctrlRegValue = (rate << (LSM303_ODR)) | _BV(LSM303_ZEN) | _BV(LSM303_YEN) | _BV(LSM303_XEN);
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG1_A, &ctrlRegValue, 1);
	
	// Set ACCEL_CTRL_REG4_A: Full-scale selection, 
	ctrlRegValue = (scale << (LSM303_FS));
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG4_A, &ctrlRegValue, 1);
	
	return 1;
}

/*
 * @see lsm303.h
 */
int lsm303_set_interrupt(uint8_t threshold, uint8_t duration) {
	uint8_t ctrlRegValue;
	
	// Enable And/Or interrupt on INT1
	ctrlRegValue = _BV(LSM303_L1_AOI1);
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG3_A, &ctrlRegValue, 1);
	
	// Latch interrupt on INT1
	ctrlRegValue = _BV(LSM303_LIR_INT1);
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_CTRL_REG5_A, &ctrlRegValue, 1);
	
	// OR combination
	// YHigh and X High
	ctrlRegValue = _BV(LSM303_YHIE_YUPE) | _BV(LSM303_XHIE_XUPE);
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_INT1_CFG_A, &ctrlRegValue, 1);
	
	// Mask the threshold so it keeps the MSB at 0
	ctrlRegValue = threshold & (0b01111111);
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_INT1_THS_A, &ctrlRegValue, 1);
	
	// Mask the duration so it keeps the MSB to 0
	ctrlRegValue = duration & (0b01111111);
	i2c_master_write(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_INT1_DURATION_A, &ctrlRegValue, 1);
	
	return 1;
}

/*
 * @see lsm303.h
 */
int lsm303_clear_latched_interrupt() {
	uint8_t readValue;
	
	// Read interrupt source -> Also clears latched interrupt
	i2c_master_read(LSM303DLHC_ADDRESS_LIN_ACCEL, LSM303_REGISTER_ACCEL_INT1_SOURCE_A, &readValue, 1);
	
	return readValue;
}

/*
 * @see lsm303.h
 */
int lsm303_read(struct lsm303_accel_reading * reading) {
	uint8_t rawReading[ACCEL_READING_SIZE];
	
	// Read status + all 6 registers in auto-increment mode for the raw register values
	i2c_master_read(LSM303DLHC_ADDRESS_LIN_ACCEL, 
					(LSM303_REGISTER_ACCEL_STATUS_REG_A | LSM303_REGISTER_AUTO_INC), 
					rawReading, ACCEL_READING_SIZE);

	reading->rawStatus = rawReading[0];
	
	// Check if the data is valid
	if (rawReading[0] & _BV(LSM303_ZYXDA)) {
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
