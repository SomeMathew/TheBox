/*
 * Library for controlling Servo motors on the atmega328p. Usage instructions are found in the header.
 */

#include "servo.h"
#include "pin_config.h"
#include "ioctl.h"
#include <avr/interrupt.h>


// ISR to set pulse to ServoB on overflow
ISR(TIMER1_OVF_vect) {
    ioctl_write(&SERVO_B_PORT, SERVO_B_IO, 1);
}

// ISR to clear pulse to ServoB on CompareMatch
ISR(TIMER1_COMPB_vect) {
    ioctl_write(&SERVO_B_PORT, SERVO_B_IO, 0);
}

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
 * Initializes the desired channel for a servo motor at neutral
 * 
 * @return 0 on success, negative on error
 */
int servo_channel_init(int channel) {
    return servo_channel_init_angle(channel, 90);
}

/**
 * Initializes the desired channel for a servo motor with the indicated
 * starting angle
 * 
 * @return 0 on success, negative on error
 */
int servo_channel_init_angle(int channel, int angle) {
    if(channel == SERVO_CHANNELA) {
        // Clear OC1x on Compare Match and set at bottom
        TCCR1A |= (1 << COM1A1);

        // Set the initial position and turn on output
        OCR1A = BASE_VALUE + ((long) angle * UNITS_PER_DEGREE);
        ioctl_setdir(&SERVO_A_DDR, SERVO_A_IO, OUTPUT);
    } else if(channel == SERVO_CHANNELB) {
        // Don't want to touch OC1B, so won't set anything for it!

        // Turn on the interrupts!
        TIMSK1 |= (1 << OCIE1B);
        TIMSK1 |= (1 << TOIE1);

        // Set the initial position and turn on output
        OCR1B = BASE_VALUE + ((long) angle * UNITS_PER_DEGREE);
        ioctl_setdir(&SERVO_B_DDR, SERVO_B_IO, OUTPUT);
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



