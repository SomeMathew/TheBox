/*
 * Simple SPI device driver for Atmega328p. Usage instructions are found in the header.
 */

#include <avr/io.h>
#include <stdint.h> 
#include <stddef.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <util/atomic.h>

#include "defineConfig.h"
#include "spi.h"

#if !defined(SPI_PORT) || !defined(SPI_DDR) || !defined(SPI_DD_SCK) || !defined(SPI_DD_MISO) || !defined(SPI_DD_MOSI) || !defined(SPI_DD_SS) || !defined(SPI_PIN)
#error SPI PINS Configuration missing, see defineConfig.h
#endif


#define SPI_CLOCKRATE_PRESCALE_4 (0x00)
#define SPI_CLOCKRATE_PRESCALE_16 (_BV(SPR0))
#define SPI_CLOCKRATE_PRESCALE_64 (_BV(SPR1))
#define SPI_CLOCKRATE_PRESCALE_128 (_BV(SPR0) | _BV(SPR1))

#define SPI_MODE_MASK (_BV(CPOL) | _BV(CPHA))
#define SPI_ORDER_MASK (_BV(DORD))
#define SPI_CLOCKRATE_MASK (_BV(SPR1) | _BV(SPR0))
#define SPI_CONTROL_MASK (_BV(SPIE) | _BV(SPE) | _BV(MSTR))

static void (*isrVector)(void) = NULL;
static volatile bool transferBusy = false;
static uint8_t* txbuffer;
static uint8_t* rxbuffer;
static volatile int txBufferRemaining = 0;
static volatile int rxBufferRemaining = 0;






static inline void setDeviceFrequency(struct spi_deviceConfig *device, uint32_t frequency) {
	
	if (frequency >= (F_CPU/2)) {
		device->controlRegister = (device->controlRegister & ~(SPI_CLOCKRATE_MASK)) | SPI_CLOCKRATE_PRESCALE_4;
		device->statusRegister = _BV(SPI2X);
	} 
	else if (frequency >= (F_CPU/4)) {
		device->controlRegister = (device->controlRegister & ~(SPI_CLOCKRATE_MASK)) | SPI_CLOCKRATE_PRESCALE_4;
	} 
	else if (frequency >= (F_CPU/8)) {
		device->controlRegister = (device->controlRegister & ~(SPI_CLOCKRATE_MASK)) | SPI_CLOCKRATE_PRESCALE_16;
		device->statusRegister = _BV(SPI2X);
	} 
	else if (frequency >= (F_CPU/16)) {
		device->controlRegister = (device->controlRegister & ~(SPI_CLOCKRATE_MASK)) | SPI_CLOCKRATE_PRESCALE_16;
	} 
	else if (frequency >= (F_CPU/32)) {
		device->controlRegister = (device->controlRegister & ~(SPI_CLOCKRATE_MASK)) | SPI_CLOCKRATE_PRESCALE_64;
		device->statusRegister = _BV(SPI2X);
	} 
	else if (frequency >= (F_CPU/64)) {
		device->controlRegister = (device->controlRegister & ~(SPI_CLOCKRATE_MASK)) | SPI_CLOCKRATE_PRESCALE_64;
	} 
	else {
		device->controlRegister = (device->controlRegister & ~(SPI_CLOCKRATE_MASK)) | SPI_CLOCKRATE_PRESCALE_128;
	}
}

/**
 * Sends the next byte from the txBuffer.
 * 
 * @returns 0 if sent, -1 if nothing to send
 */
static inline int sendNextByte() {
	if (txBufferRemaining > 0) {
		uint8_t* txp = txbuffer++; 
		txBufferRemaining--;
		return spi_write_async(*txp);
	} else {
		return -1;
	}
}

/**
 * Receives the next byte into rxBuffer.
 * 
 * @returns 0 on success, -1 if remaining was 0 (no more space)
 */
static inline int receiveNextByte() {
	if (rxBufferRemaining > 0) {
		uint8_t* rxp = rxbuffer++; 
		rxBufferRemaining--;
		return spi_read_async(rxp);
	} else {
		return -1;
	}
}


int spi_open_master(uint8_t spiControl) {
	/* Set GPIO directions */
	SPI_DDR = (SPI_DDR & ~_BV(SPI_DD_MISO)) | (_BV(SPI_DD_MOSI) | _BV(SPI_DD_SCK) | _BV(SPI_DD_SS));
	/* Reset GPIO value to 0 */
	SPI_PORT = SPI_PORT & ~(_BV(SPI_DD_MISO) | _BV(SPI_DD_MOSI) | _BV(SPI_DD_SCK) | _BV(SPI_DD_SS));
	
	/* Start SPI with the given mode */
	SPCR = spiControl;
	/* Clearing SPI interrupt flags by reading SPSR and SPDR , see datasheet */
	volatile uint8_t IOregister;
	STUB(IOregister);
	IOregister = SPSR;
	IOregister = SPDR;
	
	return 0;
}

int spi_open_slave(uint8_t spiControl, uint8_t mode, uint8_t bitOrder, void (*vector)(void)) {
	/* Set GPIO directions */
	SPI_DDR = (SPI_DDR & ~(_BV(SPI_DD_MOSI) | _BV(SPI_DD_SCK) | _BV(SPI_DD_SS))) | _BV(SPI_DD_MISO);
	/* Reset GPIO value to 0 and no pull up */
	SPI_PORT = SPI_PORT & ~(_BV(SPI_DD_MISO) | _BV(SPI_DD_MOSI) | _BV(SPI_DD_SCK) | _BV(SPI_DD_SS));
	
	// Error if we're setting interrupt mode with no vector
	if (spiControl == SPI_CONTROL_SLAVE_IT && vector == NULL) {
		return -1;
	}
	/* Start SPI as Slave in polling mode */
	SPCR = spiControl | bitOrder | mode;
	isrVector = vector;
	
	/* Clearing SPI interrupt flags by reading SPSR and SPDR , see datasheet */
	volatile uint8_t IOregister;
	STUB(IOregister);
	IOregister = SPSR;
	IOregister = SPDR;
	
	return 0;
}

int spi_ioctl_setDevice(struct spi_deviceConfig *device, uint8_t mode, uint8_t bitOrder, uint32_t frequency) {
	device->controlRegister = SPI_CONTROL_MASTER_POLL | bitOrder | mode;
	device->statusRegister = 0x00; // initialize to default before setting double speed flag in setFrequency
	
	setDeviceFrequency(device, frequency);
	
	return 0;
}


int spi_ioctl_selectDevice(struct spi_deviceConfig *device) {
	if (transferBusy == false) {
		SPCR = device->controlRegister;
		SPSR = device->statusRegister;
	
		return 0;
	} else {
		return -1;
	}
}

int spi_transmit_it(uint8_t *tx, uint8_t *rx, int size, void (*vector)(void)) {
	if (vector == NULL || transferBusy == true) {
		return -1;
	}
	txBufferRemaining = size;
	rxBufferRemaining = size;
	
	txbuffer = tx;
	rxbuffer = rx;
	
	isrVector = vector;
	
	transferBusy = true;
	return sendNextByte();
}

ISR(SPI_STC_vect) {
	// If we have a transfer complete and we're in slave IT mode 
	// then initiate the callback
	if ((SPCR & SPI_CONTROL_MASK) == SPI_CONTROL_SLAVE_IT) {
		isrVector();
	}
	// If we're in Master IT mode then continue transfer or callback if done
	else if ((SPCR & SPI_CONTROL_MASK) == SPI_CONTROL_MASTER_IT) {
		// Receive and send if there are something to do (handled by the inline functions)
		receiveNextByte();
		sendNextByte();
		
		if (rxBufferRemaining <= 0 && txBufferRemaining <= 0) {
			transferBusy = false;
			isrVector();
		}
	}
}
