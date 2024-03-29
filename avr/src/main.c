#include <avr/io.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <stdint.h>
#include "main.h"
#include "uart.h"
#include "defineConfig.h"
#include "command.h"
#include "box_control.h"
#include "servo.h"
#include "bbb_commands.h"
#include "spi_command.h"
#include "lsm303.h"
#include "i2c.h"
#include "ioctl.h"
#include "pin_config.h"
#include "alert.h"

#define SERIAL_INPUT_BUFFER_SIZE 32

static void setup();
static void loop();
static void processSerialInput(void);

static void pong(char *);
static void moveA(char *);
static void moveB(char *);
static void readAccel(char *);
static void sendToBBB(char *);
static void clearAccelInt(char *);
static void alertstatus(char *);

static void isOpen();
static void bbbOpen();
static void bbbClose();

/**
 * Lists of UART command vectors used to debug and manually operate
 * the system.
 */
static struct Command optList[] = {
  {"ping", pong, true}, // Alive check and debug
  {"moveA", moveA, true},
  {"moveB", moveB, true},
  {"isOpen", isOpen, false},
  {"bbbOpen", bbbOpen, false},
  {"bbbClose", bbbClose, false},
  {"sendbbb", sendToBBB, true},
  {"ra", readAccel, false},
  {"cai", clearAccelInt, false},
  {"alert", alertstatus, false}
}; 

int main() {
	setup();

	while(1) {
		loop();
	}
	return 0;
}

void setup() {
	uart_open(UART_BAUD_RATE, UART_DIRECTION, UART_PARITY, UART_FRAME_SIZE, UART_STOPBIT);
	sei();
	
	fprintf(&uartStream, "Init cmd...\n");
	command_setup(optList, LENGTH_OF_ARRAY(optList));
	
	fprintf(&uartStream, "Init spicmd...\n");
	spicmd_init();
	
	fprintf(&uartStream, "Init box...\n");
	box_init();
		
	fprintf(&uartStream, "Init i2c...\n");
	i2c_master_init(I2C_FREQUENCY);
	
	fprintf(&uartStream, "Init alert...\n");
	alert_init();
	
	fprintf(&uartStream, "Init global GPIOs...\n");	
	ioctl_setdir(&LED_ALIVE_DDR, LED_ALIVE_IO, OUTPUT);
	ioctl_setdir(&ACCEL_INT_DDR, ACCEL_INT_DDR, INPUT); 
	
	fprintf(&uartStream, "System ready!\n");
}

void loop() {
	processSerialInput();
	box_handleCurrentState();
	alert_run(box_isOpen() ? ALERT_RUN_DISARM : ALERT_RUN_ARMED);
}

/**
 * Reads the Serial Buffer if an input is available until a newline or the buffer is full, then execute any found commands.
 */
static void processSerialInput(void) {
	static char inputBuffer[SERIAL_INPUT_BUFFER_SIZE];
	static size_t inputBufferNextPos = 0;

	while (uart_available() > 0) {
		char nextChar = uart_read();
		if (inputBufferNextPos == SERIAL_INPUT_BUFFER_SIZE - 1 || nextChar == '\n') {
			// Execute the command if its a newline or we're about to overflow the buffer
			inputBuffer[inputBufferNextPos] = '\0';
			command_execute(inputBuffer, NULL);
			inputBufferNextPos = 0;
		} else if (nextChar != '\r') { // We disregard \r for simplicity and compatibility with different newline standards
			inputBuffer[inputBufferNextPos++] = nextChar;
		}
	}
}

/**
 * Sends a command to the BBB using SPI and the Status GPIO.
 */
static void sendToBBB(char * arg) {
	uint8_t i = atoi(arg);
	fprintf(&uartStream, "sending! %" PRIx8 "\n", i);
	spicmd_send(i);
}

/**
 * Displays the current raw alert status to UART.
 */
static void alertstatus(char * arg) {
	fprintf(&uartStream, "Status: %"PRIx8 "\n", alert_getstatus());
}

/**
 * Reads and displays the accelerometer reading to UART.
 */
static void readAccel(char * arg) {
	struct lsm303_accel_reading reading;
	
	lsm303_read(&reading);
	fprintf(&uartStream, "Accel: status: %"PRIx8", x: %"PRId16" y: %"PRId16" z: %"PRId16"\n", reading.rawStatus, reading.x, reading.y, reading.z); 
}


/**
 * Clears and displays the accelerometer interrupt status.
 */
static void clearAccelInt(char * arg) {
	uint8_t src = lsm303_clear_latched_interrupt();
	
	fprintf(&uartStream, "Int SRC: %"PRIx8"\n", src);
}


/**
 * Responds to a ping request.
 */
static void pong(char * arg) {
	uint8_t i = atoi(arg);
	fprintf(&uartStream, "Pong! %" PRIu8 "\n", i);
}

/**
 * Moves servo on channel A to desired angle. This is lid motor.
 */
static void moveA(char * arg) {
	int i = atoi(arg);
	fprintf(&uartStream, " Moving Servo A %d\n", i);
	servo_write(SERVO_CHANNELA, i);
}

/**
 * Moves servo on channel B to desired angle. This is lock motor.
 */
static void moveB(char * arg) {
	int i = atoi(arg);
	fprintf(&uartStream, " Moving Servo B %d\n", i);
	servo_write(SERVO_CHANNELB, i);
}

/**
 * Prints to UART if the box is opened.
 */
static void isOpen() {
	fprintf(&uartStream, "Is switch open? %d\n", box_isOpen());
	// spicmd_callback_checkstatus();
}

/**
 * Unlocks and opens the box.
 */
static void bbbOpen() {
	spicmd_callback_unlockopen();
}

/**
 * Closes and locks the box
 */
static void bbbClose() {
	spicmd_callback_closelock();
}
