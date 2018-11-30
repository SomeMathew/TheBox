#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "i2c.h"

static uint8_t txBuffer[I2C_TX_BUFFER_SIZE];

static volatile size_t txBufferTail = 0;
static volatile size_t txBufferHead = 0;

static volatile uint8_t status;

static void (*slarVector)(void);

/**
 * Waits for the TWINT flag (Completion). 
 * Used for polling mode
 */
static inline void i2c_waitForComplete(void) {
	loop_until_bit_is_set(TWCR, TWINT);
}

/**
 * Send a start condition on the bus
 */
static inline void i2c_start(void) {
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTA);
}

/**
 * Sends a stop condition on the bus
 */
static inline void i2c_stop(void) {
	TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
}

/**
 * Reads from the bus with an ACK. (Blocking)
 * 
 * @return Byte reading
 */
static inline uint8_t i2c_readAck(void) {
	TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA);
	i2c_waitForComplete();
	return TWDR;
}

/**
 * Reads a byte from the bus without any following ACK. (Blocking)
 *
 * @return Byte reading
 */
static inline uint8_t i2c_readNoAck(void) {
	TWCR = _BV(TWEN) | _BV(TWINT);
	i2c_waitForComplete();
	return TWDR;
}

/**
 * Send a byte with a NoAck. (Blocking)
 * 
 * @param data byte to send
 */
static inline void i2c_sendNoAck(uint8_t data) {
	TWDR = data;
	TWCR = _BV(TWINT) | _BV(TWEN);    
	i2c_waitForComplete();
}

int i2c_master_init(uint32_t frequency) {
	// No prescaler
	TWSR &= ~(_BV(TWPS1) | _BV(TWPS0));
	// SCLFreq = F_CPU / (16+2*TWBR*Prescaler)
	TWBR = (F_CPU / (2*frequency)) - 8;
	TWCR = _BV(TWEN);
	
	return 0;
}

int i2c_slave_init(uint8_t addr8) {
	TWAR = addr8 & ~_BV(TWGCE);
	TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
	
	return 0;
}

int i2c_master_receive(uint8_t addr8, uint8_t *dataBuffer, size_t size) {
	i2c_start();
	i2c_waitForComplete();
	if(TW_STATUS != TW_START || TW_STATUS != TW_REP_START) {
		status = TW_STATUS;
		return -1; 
	}

	i2c_sendNoAck(addr8 | _BV(0)); // SLA+R
	
	for (int i = 0; i < size; i++) {
		dataBuffer[i] = (i < size-1) ? i2c_readAck() : i2c_readNoAck();
	}

	i2c_stop();
	
	return 0;
}

/**
 * Read a register from the slave device.
 * This is a blocking implementation.
 * 
 * @param addr8 Address of the Slave to read, in 8 bit format.
 * @param reg Register to read from the device.
 * @param dataBuffer Buffer to receive the data to
 * @param size Size of the transmition
 */
int i2c_master_read(uint8_t addr8, uint8_t reg, uint8_t *dataBuffer, size_t size) {
	i2c_start();
	i2c_waitForComplete();
		if(TW_STATUS != TW_START) {
		status = TW_STATUS;
		return -1; 
	}
	i2c_sendNoAck(addr8 & ~_BV(0)); // SLA+W
	i2c_sendNoAck(reg);
	// Now receive from the register, will issue a repeated start
	return i2c_master_receive(addr8, dataBuffer, size); 
}

/**
 * Write a register of the slave device.
 * This is a blocking implementation.
 * 
 * @param addr8 Address of the Slave to write, in 8 bit format.
 * @param reg Register to read to on the device.
 * @param dataBuffer Buffer to read the data from
 * @param size Size of the transmition
 */
int i2c_master_write(uint8_t addr8, uint8_t reg, uint8_t *dataBuffer, size_t size) {
	i2c_start();
	i2c_waitForComplete();
		if(TW_STATUS != TW_START) {
		status = TW_STATUS;
		return -1; 
	}
	i2c_sendNoAck(addr8 | _BV(1)); // SLA+W
	i2c_sendNoAck(reg);
	
	for (int i = 0; i < size; i++) {
		i2c_sendNoAck(dataBuffer[i]);
	}

	i2c_stop();
	
	return 0;
}

int i2c_slave_transmit(uint8_t *data, size_t size) {
	int remaining = (txBufferTail - txBufferHead + I2C_TX_BUFFER_SIZE - 1) % I2C_TX_BUFFER_SIZE;
	
	if (remaining < size) {
		return -1;
	}

	int i;
	for(i = 0; i < size; i++) {
		txBuffer[txBufferHead + i] = data[i]; 
	}
	txBufferHead = (txBufferHead + size) % I2C_TX_BUFFER_SIZE;
	return i;
}

int i2c_attachIrq_slave_read_recv(void (*vector)(void)) {
	slarVector = vector;
	return 0;
}

/**
 * Handling of the I2C state machine for interrupt based operations.
 * Currently only the Slave Transmitter function is implemented with
 * interrupts.
 */
ISR(TWI_vect) {
	// Handle the I2C state machine for Slave Transmitter
	switch(TW_STATUS) {
		// SLA+R Received
		case TW_ST_SLA_ACK:
			slarVector();
		// Data transmitted, Ack received
		case TW_ST_DATA_ACK:
			TWDR = txBuffer[txBufferTail++];
			if (txBufferTail != txBufferHead) {
				TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWIE) | _BV(TWEA);
			} else {
				TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWIE);
			}
			break;
		// Data transmitted Nack received
		case TW_ST_DATA_NACK:
		// Last data transmitted but Ack received
		case TW_ST_LAST_DATA:
			txBufferTail = 0;
			txBufferHead = 0;
			TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWIE) | _BV(TWEA);
			break;
	}
}
