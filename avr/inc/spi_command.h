#ifndef _DEV_SPI_CMD_H
#define _DEV_SPI_CMD_H

#define SPICMD_BBB_ALERT 	(0xB1)
#define SPICMD_BBB_CLOSED 	(0xB2)
#define SPICMD_BBB_OPENED 	(0xB3)

#define SPICMD_RESP_OPENED 	(0xEA)
#define SPICMD_RESP_CLOSED 	(0xEB)

#define SPICMD_ACK 			(0xFA)
#define SPICMD_NACK 		(0xFB)


#define SPICMD_OK 				(0)
#define SPICMD_ERR_BUSY 		(1)
#define SPICMD_ERR_UNEXPECTED 	(2)
#define SPICMD_ERR_NOTINIT 		(3)


/**
 * Initializes the module and the attached SPI interface
 * 
 * @returns SPICMD_OK or SPICMD_ERR_*
 */
int spicmd_init();

/**
 * Send a status to the attached device through the status GPIO feedback.
 * 
 * @param cmd SPICMD_BBB_* or SPICMD_RESP_*
 */
int spicmd_send(uint8_t cmd);

int spicmd_callback_unlockopen() __attribute__((weak, alias("spi_cmd_none")));
int spicmd_callback_closelock() __attribute__((weak, alias("spi_cmd_none")));
int spicmd_callback_checkstatus() __attribute__((weak, alias("spi_cmd_none")));

#endif /* _DEV_SPI_CMD_H */
