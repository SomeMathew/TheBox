#include <stdint.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/atomic.h>
#include <stddef.h>
#include "spi_command.h"
#include "pin_config.h"
#include "spi.h"
#include "ioctl.h"

#define OUTPUT_BUFFER_SIZE 8

#define STATE_OFF 			(0)
#define STATE_WAIT 			(1)
#define STATE_CMD_SENT 		(2)
#define STATE_ACKED 		(6)

#define CMD_IN_UNLOCK_OPEN 	(0xA1)
#define CMD_IN_LOCK_CLOSE 	(0xA2)
#define CMD_IN_CHECK_STATUS (0xA3)

#define CMD_IN_GET_STATUS	(0xC1)

typedef uint8_t State;

static void spiVector(void);
static void pullLowFromTriState(void);
static void prepareWaitingCommand(void);
static int addToBuffer(uint8_t c);
static void checkAndCallVector(uint8_t cmd);

static uint8_t outputBuffer[OUTPUT_BUFFER_SIZE];
static volatile size_t outputBufferHead = 0;
static volatile size_t outputBufferTail = 0;

static volatile State state = STATE_OFF; 

static inline bool commandBufferIsEmpty() {
	return outputBufferHead == outputBufferTail;
}

/**
 * @see spi_command.h
 */
int spicmd_init() {
	ioctl_tristate(&BBB_STATUS_DDR, &BBB_STATUS_PORT, BBB_STATUS_IO);
	spi_open_slave(SPI_CONTROL_SLAVE_IT, SPI_MODE_0, SPI_ORDER_MSB_FIRST, spiVector);
	state = STATE_WAIT;
	
	return SPICMD_OK;
}

/**
 * @see spi_command.h
 */
int spicmd_send(uint8_t cmd) {
	int status = addToBuffer(cmd);
	pullLowFromTriState();
	return status;
}

/**
 * Pull the BBB Status GPIO low from the tri state mode.
 * 
 * The BBB Status GPIO is active low with a pull-up
 */ 
static void pullLowFromTriState() {
	ioctl_setdir(&BBB_STATUS_DDR, BBB_STATUS_IO, OUTPUT);
	ioctl_write(&BBB_STATUS_PORT, BBB_STATUS_IO, 0);
}

/**
 * Add a command to the buffer to send to the BBB.
 * 
 * This queues the command to be shifted by the BBB when CMD_IN_GET_STATUS
 * is received by the interface.
 * 
 * @param c The command to send. 
 */
static int addToBuffer(uint8_t c) {
	size_t i = (outputBufferHead + 1) % OUTPUT_BUFFER_SIZE;

	// Fail if buffer is full
	if (i == outputBufferTail) {
		return SPICMD_ERR_BUSY;
	}

	outputBuffer[outputBufferHead] = c;

	ATOMIC_BLOCK(ATOMIC_FORCEON) {
	  outputBufferHead = i;
	}

	return SPICMD_OK;
}

/**
 * Interrupt vector callback for the shift event of the SPI driver.
 * 
 * This function is the main workhorse of this module it controls the
 * state changes and handles shifting the correct value to the 
 * SPI buffer when an event is received.
 */
static void spiVector() {
	uint8_t recv = 0;
	
	switch (state) {
		// We just sent a cmd, this is the shift event from the master
		// Go back to waiting in tristate if no other commands are available
		case STATE_CMD_SENT:
			if (commandBufferIsEmpty()) {
				ioctl_tristate(&BBB_STATUS_DDR, &BBB_STATUS_PORT, BBB_STATUS_IO);
			}
		// Shift of the Ack and falldown that need to go back to wait
		case STATE_ACKED:
			state = STATE_WAIT;
			break;
		
		// New Command from SPI
		case STATE_WAIT:
			spi_read_async(&recv);
			if (recv == CMD_IN_GET_STATUS) {
				prepareWaitingCommand();
			} else {
				checkAndCallVector(recv);
			}
			break;
	}
}

/**
 * Prepare the next waiting command in the buffer to the SPI buffer.
 * 
 * This is an helper function to the spiVector entry point.
 * 
 * Atomic operations are not necessary since it is designed to be called
 * in a non-nested ISR.
 */
static void prepareWaitingCommand() {
	if (!commandBufferIsEmpty()) {
		uint8_t nextCmd = outputBuffer[outputBufferTail];
		outputBufferTail = (outputBufferTail + 1) % OUTPUT_BUFFER_SIZE;
		spi_write_async(nextCmd);
		state = STATE_CMD_SENT;
	} else {
		spi_write_async(SPICMD_NACK);
		state = STATE_WAIT;
	}
}

/**
 * Validates the command received from the SPI interface.
 * 
 * If it is a valid command it will execute the associated vector callback.
 * 
 * @param cmd The byte command received from the SPI interface.
 */
static void checkAndCallVector(uint8_t cmd) {
	int (*vector)() = NULL;
	switch(cmd) {
		case CMD_IN_UNLOCK_OPEN:
			vector = spicmd_callback_unlockopen;
			break;
		case CMD_IN_LOCK_CLOSE:
			vector = spicmd_callback_closelock;
			break;
		case CMD_IN_CHECK_STATUS:
			vector = spicmd_callback_checkstatus;
			break;
	}
	
	if (vector != NULL) {
		spi_write_async(SPICMD_ACK);
		vector();
	} else {
		spi_write_async(SPICMD_NACK);
	}
}

/**
 * Stub alias for the weak function vector declared in the header.
 * 
 * This will be called if the callback are not reimplemented for use.
 */
int spicmd_callback_unlockopen(void) {
	return -1;
}

/**
 * Stub alias for the weak function vector declared in the header.
 * 
 * This will be called if the callback are not reimplemented for use.
 */
int spicmd_callback_closelock(void) {
	return -1;
}

/**
 * Stub alias for the weak function vector declared in the header.
 * 
 * This will be called if the callback are not reimplemented for use.
 */
int spicmd_callback_checkstatus(void) {
	return -1;
}
