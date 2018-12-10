#ifndef _DEV_SPI_CMD_H
#define _DEV_SPI_CMD_H

#define SPICMD_BBB_ALERT 	(0xB1)

#define SPICMD_RESP_OPENED 	(0xEA)
#define SPICMD_RESP_CLOSED 	(0xEB)

#define SPICMD_ACK 			(0xFA)
#define SPICMD_NACK 		(0xFB)


#define SPICMD_OK 				(0xFA)
#define SPICMD_ERR_BUSY 		(0xFD)
#define SPICMD_ERR_UNEXPECTED 	(0xFB)
#define SPICMD_ERR_NOTINIT 		(0xFC)


/**
 * Initializes the spi_command module for the interface between the BBB
 * and the AVR.
 * 
 * This initializes the Status GPIO to tristated and initializes the
 * SPI peripheral as slave.
 * 
 * @returns SPICMD_OK or SPICMD_ERR_*
 */
int spicmd_init();

/**
 * Send a status or command to the attached device through the status 
 * GPIO feedback.
 * 
 * @param cmd SPICMD_BBB_* or SPICMD_RESP_*
 * 
 * @return SPICMD_OK or SPICMD_ERR_*
 */
int spicmd_send(uint8_t cmd);


int spicmd_callback_unlockopen(void) __attribute__((weak));
int spicmd_callback_closelock(void) __attribute__((weak));
int spicmd_callback_checkstatus(void) __attribute__((weak));

#endif /* _DEV_SPI_CMD_H */
