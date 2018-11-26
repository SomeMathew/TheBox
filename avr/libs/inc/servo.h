/**
 * Library to control servo motors using the atmega328P. In order to use, the library
 * must be initialized as follows:
 * 1) Call servo_init() to setup timers for the servo motors
 * 2) Initialize the desired channel using servo_channel_init(channel)
 */
#ifndef _DEV_SERVO_H
#define _DEV_SERVO_H

#include <avr/io.h>

#define SERVO_CHANNELA 0
#define SERVO_CHANNELB 1

#define NEUTRAL 375
#define BASE_VALUE 150
#define MIN_ANGLE 0
#define MAX_ANGLE 180
#define UNITS_PER_DEGREE 2.5

/**
 * Initializes the Servo library by setting up Timer1
 * 
 * @return 0 on success, negative on error
 */
int servo_init();

/**
 * Initializes the desired channel for a servo motor
 * 
 * @return 0 on success, negative on error
 */
int servo_channel_init(int channel);

/**
 * Moves the requested servo motor to the desired angle. Must be a value between 0 and 180
 * 
 * @param channel The Servo to be written to
 * @param angle The angle to move to (between 0 and 180)
 * @return 0 on success, negative on error
 */
int servo_write(int channel, int angle);

/**
 * Returns the current angle of the servo on the desired channel
 * 
 * @param channel The servo to read
 * @return the angle of the motor, between 0 and 180
 */
long servo_read(int channel);

#endif