/**
 * SPI driver for atmega328P, Master mode only. The SPI bus must be first initialized with a call to
 * spi_open(), and each device must have a struct deviceConfig initialized with spi_ioctl_setDevice(). 
 * Before a write transaction the bus must be configured with the given device's config struct 
 * using ioctl_setBus(). 
 * 
 * At present this driver provides limited support for multiple device on the bus. If needed, a lock
 * mechanism must be handled by the application program.
 */
 
#ifndef _DEV_SPI_H
#define _DEV_SPI_H


#include <stdint.h>
#include <avr/io.h>
#include <stdbool.h>

#include "defineConfig.h"

#define SPI_CONTROL_MASTER_POLL (_BV(SPE) | _BV(MSTR)) // polling Master
#define SPI_CONTROL_MASTER_IT (_BV(SPIE) | _BV(SPE) | _BV(MSTR))
#define SPI_CONTROL_SLAVE_POLL (_BV(SPE)) // Polling Slave
#define SPI_CONTROL_SLAVE_IT (_BV(SPIE) | _BV(SPE))

#define SPI_MODE_0 (0x00) // CPOL 0, CPHA 0
#define SPI_MODE_1 (_BV(CPHA)) // CPOL 0, CPHA 1
#define SPI_MODE_2 (_BV(CPOL)) // CPOL 1, CPHA 0
#define SPI_MODE_3 ((_BV(CPOL)) | (_BV(CPHA)))   // CPOL 1, CPHA 1

#define SPI_ORDER_MSB_FIRST (0x00)
#define SPI_ORDER_LSB_FIRST (_BV(DORD))

struct spi_deviceConfig {	
	/* Private */
	uint8_t controlRegister;
	uint8_t statusRegister;
};

/**
 * Activate the spi clock signal and set the required GPIO for Master Mode.
 * 
 * The SPI bus pins are set in defineConfig.h
 * MOSI : Output
 * MISO : Input
 * SCK  : Output
 * SS   : Output ( If changed the SPI will switch to a slave when driven low )
 * 
 * @param spiControl SPI_CONTROL_MASTER_xxx Define
 * @return negative on error
 */
int spi_open_master(uint8_t spiControl);

/**
 * Activate the spi and the required GPIO for Slave Mode.
 * 
 * The SPI bus pins are set in defineConfig.h
 * MOSI : Input
 * MISO : Output
 * SCK  : Input
 * SS   : Input
 * 
 * @param spiControl SPI_CONTROL_SLAVE_xxx Define
 * @param mode One of the define SPI Mode
 * @param bitOrder One of the define bit order
 * @param vector Interrupt function called when transmition is completed 
 * 			(Must be set if spiControl is IT)
 * @return negative on error
 */
int spi_open_slave(uint8_t spiControl, uint8_t mode, uint8_t bitOrder, void (*vector)(void));

/**
 * Initialize and configure a device structure to use with setBus before a transmission. 
 * 
 * @param device Pointer to a created struct that will be set by this function 
 * @param mode One of the define SPI Mode
 * @param bitOrder One of the define bit order
 * @param frequency Frequency of the clock
 */
int spi_ioctl_setDevice(struct spi_deviceConfig *device, uint8_t mode, uint8_t bitOrder, uint32_t frequency);

/**
 * Set the SPI bus for a given device deviceConf struct initialized using getDeviceConfig. Must be
 * used before a transmission is made to the chosen device.
 * 
 * @param device Pointer to an initialize device struct.
 * @returns 0 on success, -1 if busy
 */
int spi_ioctl_selectDevice(struct spi_deviceConfig *device);

/**
 * Start a read/write transaction with the last device initialized with spi_ioctl_setDevice.
 * 
 * The SS of the chosen device must be controlled by the application side.
 * This is a non-blocking function using interrupt, when data is fully transmitted 
 * the vector will be called.
 * 		The vector is called during an interrupt, it should be exited quickly.
 * 
 * @param tx Buffer of bytes to transmit
 * @param rx Buffer to receive bytes
 * @param size Size to transmit and receive
 * @param vector Callback when transmit is completed.
 * 
 * @returns 0 if successfully called, -1 if the device is busy
 */
int spi_transmit_it(uint8_t *tx, uint8_t *rx, int size, void (*vector)(void));



/**
 * Start a read/write transaction with the last device initialized with spi_ioctl_setDevice.
 * 
 * The chip select of the chosen device must be controlled by the application side. 
 * 
 * @param tx Byte to send out
 * @param rx Pointer to byte to receive
 */
static inline int spi_transmit(uint8_t tx, uint8_t *rx) {
	SPDR = tx;
	loop_until_bit_is_set(SPSR, SPIF);
	*rx = SPDR;
	
	return 0;
}

/**
 * Start a read/write transaction in blocking mode.
 * 
 * This function will return if the SS is set to High during transmission.
 * 
 * @param tx Byte to send out
 * @param rx Pointer to byte to receive
 */
static inline int spi_transmit_slave(uint8_t tx, uint8_t *rx) {
	SPDR = tx;
	while(bit_is_clear(SPSR, SPIF) && bit_is_clear(SPI_PIN, SPI_PIN_SS)) {};
	*rx = SPDR;
	return 0;
}

/**
 * Read/Write asynchronously, this is used for IT based comm by the slave.
 * 
 * To be used during the vector callback
 * 
 * @param tx Byte to send out
 * @param rx points to read received byte
 */
static inline int spi_transmit_async(uint8_t tx, uint8_t *rx) {
	SPDR = tx;
	return 0;
}

/**
 * Write asynchronously, this is used for IT based comm by the slave.
 * 
 * To be used during the vector callback
 * 
 * @param tx Byte to send out
 */
static inline int spi_write_async(uint8_t tx) {
	SPDR = tx;
	return 0;
}

/**
 * Read asynchronously, this is used for IT based comm by the slave.
 * 
 * To be used during the vector callback
 * 
 * @param rx points to read received byte
 */
static inline int spi_read_async(uint8_t *rx) {
	*rx = SPDR;
	return 0;
}
#endif /* _DEV_SPI_H */
