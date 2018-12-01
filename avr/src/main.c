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

#define SERIAL_INPUT_BUFFER_SIZE 32

static void setup();
static void loop();
static void processSerialInput(void);

static void pong(char *);
static void moveA(char *);
static void moveB(char *);

static void open();
static void isOpen();
static void close();
static void lock();
static void unlock();

static void bbbOpen();
static void bbbClose();

static void sendToBBB(char *);

static struct Command optList[] = {
  {"ping", pong, true}, // Alive check and debug
  {"moveA", moveA, true},
  {"moveB", moveB, true},
  {"open", open, false},
  {"isOpen", isOpen, false},
  {"close", close, false},
  {"lock", lock, false},
  {"unlock", unlock, false},
  {"bbbOpen", bbbOpen, false},
  {"bbbClose", bbbClose, false},
  {"sendbbb", sendToBBB, true},
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
	command_setup(optList, LENGTH_OF_ARRAY(optList));
	spicmd_init();
	box_init();
	spicmd_init();
	sei();
}

void loop() {
	processSerialInput();
	box_handleCurrentState();
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

static void sendToBBB(char * arg) {
	uint8_t i = atoi(arg);
	fprintf(&uartStream, "sending! %" PRIx8 "\n", i);
	spicmd_send(i);
}

//~ int spicmd_callback_closelock(void) {
	//~ fprintf(&uartStream, "closelock called\n");
	//~ return 1;
//~ }

//~ int spicmd_callback_checkstatus(void) {
	//~ fprintf(&uartStream, "checkstatus called\n");
	//~ return 1;
//~ }


//~ int spicmd_callback_unlockopen(void) {
	//~ fprintf(&uartStream, "unlockopen called\n");
	//~ return 1;
//~ }

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
	servo_write(SERVO_CHANNELA, i);
}

/**
 * Moves servo on channel B to desired angle. This is lock motor.
 */
static void moveB(char * arg) {
	int i = atoi(arg);
	fprintf(&uartStream, " Moving %d\n", i);
	servo_write(SERVO_CHANNELB, i);
}

static void open() {
	box_open();
}

/**
 * Moves servo on channel A to desired angle
 */
static void isOpen() {
	fprintf(&uartStream, "Is switch open? %d\n", box_isOpen());
	// spicmd_callback_checkstatus();
}

static void close() {
	box_close();
}

static void lock() {
	box_lock();
}

static void unlock() {
	box_unlock();
}

static void bbbOpen() {
	spicmd_callback_unlockopen();
}

static void bbbClose() {
	spicmd_callback_closelock();
}
