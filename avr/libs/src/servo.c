/*
 * Library for controlling Servo motors on the atmega328p. Usage instructions are found in the header.
 */

#include "servo.h"
#include <avr/io.h>

/**
 * Initializes the Servo library by setting up Timer1
 * 
 * @return 0 on success, negative on error
 */
int servo_init() {

    // Use fast PWM, non-inverted with ICR1 as top 
    TCCR1B |= (1 << WGM13) |  (1 << WGM12);
    TCCR1A |= (1 << WGM11);

    // Set prescalar to 64
    TCCR1B |= (1 << CS11) | (1 << CS10);

    // Set ICR1 (TOP) to 5000 - b/c we increment counter at 250 000 Hz, want 50 Hz frequency
    // giving us 250 000/50 = 5000
    ICR1 = 5000;

    // Finally, we'll set the initial value of the counter
    TCNT1 = 0;

    return 0;
}

/**
 * Initializes the desired channel for a servo motor
 * 
 * @return 0 on success, negative on error
 */
int servo_channel_init(int channel) {
    if(channel == SERVO_CHANNELA) {
        // Clear OC1x on Compare Match and set at bottom
        TCCR1A |= (1 << COM1A1);

        // Set the initial position to Neutral, and turn on output
        OCR1A = NEUTRAL;
        DDRB |= (1 << PB1); // This corresponds to OC1A, the pin labelled D9 on the breadboard
    } else if(channel == SERVO_CHANNELB) {
        // Clear OC1x on Compare Match and set at bottom
        TCCR1A |= (1 << COM1B1);

        // Set the initial position to Neutral, and turn on output
        OCR1B = NEUTRAL;
        DDRB |= (1 << PB2); // This corresponds to OC1B, the pin labelled D9 on the breadboard
    } else {
        // Invalid channel, return -1
        return -1;
    }

    return 0;
}

/**
 * Moves the requested servo motor to the desired angle. Must be a value between 0 and 180
 * 
 * @param channel The Servo to be written to
 * @param angle The angle to move to (between 0 and 180)
 * @return 0 on success, negative on error
 */
int servo_write(int channel, int angle) {
    // Check to ensure that angle is valid
    if(angle < MIN_ANGLE || angle > MAX_ANGLE) {
        return -1;
    }
    
    // Update the channel to the desired angle
    if(channel == SERVO_CHANNELA) {
        OCR1A = BASE_VALUE + ((long) angle * UNITS_PER_DEGREE);
    } else if(channel == SERVO_CHANNELB) {
        OCR1B = BASE_VALUE + (angle * UNITS_PER_DEGREE);
    } else {
        // Invalid channel, return -1
        return -1;
    }

    return 0;
}

/**
 * Returns the current angle of the servo on the desired channel
 * 
 * @param channel The servo to read
 * @return the angle of the motor, between 0 and 180
 */
long servo_read(int channel) {
    if(channel == SERVO_CHANNELA) {
        return (long) (OCR1A - BASE_VALUE) / UNITS_PER_DEGREE;
    } else if (channel == SERVO_CHANNELB) {
        return (long) (OCR1B - BASE_VALUE) / UNITS_PER_DEGREE;
    } else {
        return -1;
    }
}



