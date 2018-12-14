#ifndef _DEV_I2C_H
#define _DEV_I2C_H

#include <stddef.h>

#if !defined(I2C_TX_BUFFER_SIZE)
#define I2C_TX_BUFFER_SIZE 32
#endif


/**
 * Initializes the i2c bus as Master with the given frequency.
 * 
 * F_CPU define must be set to the correct value for the CPU clock.
 * Consult the documentation for minimum or maximum. This function uses
 * the formula from the datasheet and doesn't do any sanitation
 * of the frequency.
 * 
 * 	Formula: From Atmega 328p Datasheet section 26.5.2
 * 			SCL freq = F_CPU / (16 + 2(TWBR)*(PrescalerValue)
 * 
 * 			PrescalerValue is set to 1 (No prescaler) 
 */
int i2c_master_init(uint32_t frequency);

/**
 * Initializes the i2c bus as slave with the given address.
 * 
 * @param addr Address of this slave (8 bit format).
 */
int i2c_slave_init(uint8_t addr8);

/**
 * Transmit data in Slave Transmitter Mode.
 * 
 * @param data Pointer to data buffer
 * @param size Size of the data
 * 
 * @returns -1 on error (If buffer is too full)
 */
int i2c_slave_transmit(uint8_t *data, size_t size);

/**
 * Receive data in Master Receiver Mode.
 * This is a blocking implementation.
 * 
 * @param addr8 Address of the Slave to read, in 8 bit format.
 * @param dataBuffer Buffer to receive the data to
 * @param size Size of the transmition
 */
int i2c_master_receive(uint8_t addr8, uint8_t *dataBuffer, size_t size);

/**
 * Transmit data in Master Transmitter Mode.
 * This is a blocking implementation.
 * 
 * @param addr8 Address of the Slave to write, in 8 bit format.
 * @param dataBuffer Buffer to read the data from
 * @param size Size of the transmition
 * 
 * @returns -1 on error
 */
int i2c_master_transmit(uint8_t addr8, uint8_t *dataBuffer, size_t size);


/**
 * Read a register from the slave device.
 * This is a blocking implementation.
 * 
 * @param addr8 Address of the Slave to read, in 8 bit format.
 * @param reg Register to read from the device.
 * @param dataBuffer Buffer to receive the data to
 * @param size Size of the transmition
 */
int i2c_master_read(uint8_t addr8, uint8_t reg, uint8_t *dataBuffer, size_t size);

/**
 * Write a register of the slave device.
 * This is a blocking implementation.
 * 
 * @param addr8 Address of the Slave to write, in 8 bit format.
 * @param reg Register to read to on the device.
 * @param dataBuffer Buffer to read the data from
 * @param size Size of the transmition
 */
int i2c_master_write(uint8_t addr8, uint8_t reg, uint8_t *dataBuffer, size_t size);


/**
 * Attach an interrupt vector to be called when configured as slave and that
 * the device SLA+R is called.
 * 
 * This interrupt should be used to then feed data to the i2c with 
 * i2c_slave_transmit(...)
 * 
 * @param vector Function pointer to handle the interrupt
 */
int i2c_attachIrq_slave_read_recv(void (*vector)(void));

#endif /* _DEV_I2C_H */
