/**
 * This module implements the communication interface between the
 * AVR and the BBB.
 * 
 * It uses a combination of SPI and an active low tri-stated GPIO
 * to signal available command or status to the BBB. These can then
 * be clocked out of the AVR by the BBB when convenient.
 * 
 * This implementation requires the use of the spi.h device driver
 * and uses interrupts to manage the SPI peripheral as Slave.
 * 
 * Required GPIO definition in pin_config.h: 
 * 		BBB_STATUS_DDR, BBB_STATUS_PORT, BBB_STATUS_PIN, BBB_STATUS_IO
 */

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

#define OUTPUT_BUFFER_SIZE 8


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

int spicmd_callback_unlockopen(void);
int spicmd_callback_closelock(void);
int spicmd_callback_checkstatus(void);

#endif /* _DEV_SPI_CMD_H */
