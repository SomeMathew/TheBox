#!/usr/bin/python

import Adafruit_BBIO.GPIO as GPIO
import time
import Adafruit_BBIO.SPI as SPI
import sys
import subprocess
from usb_cam import take_picture_and_recognize
from send_email import send_alert

from Adafruit_BBIO.SPI import SPI

#Bash Script to cofigure pins for SPI
subprocess.call(['/var/lib/cloud9/theBox/spi.sh'])


# spi = SPI(bus, device) #/dev/spidev<bus>.<device>
# spi = SPI (0,0) #dev/spidev1.0
# spi = SPI (0,1) #dev/spidev1.1
# spi = SPI (1,0) #dev/spidev2.0
# spi = SPI (1,1) #dev/spidev2.1

# setting up the statuses to know if box is opened or closed, right now just GPIO
opened = 0xEA
closed = 0xEB
statusGPIO = 0xC1
alertStatus = 0xB1
openBox = 0xA1
closeBox = 0xA2

# spi set up
spi = SPI(1, 0)
spi.mode = 0
spi.msh = 1000000


# initialization
def bbb_init():
    # setting up the button pin as input
    GPIO.setup("P8_12", GPIO.IN)
    # setting up the green LED as output
    GPIO.setup("P8_10", GPIO.OUT)
    # setting up the red LED as output
    GPIO.setup("P8_11", GPIO.OUT)
    # setting up the yellow LED as output
    GPIO.setup("P8_14", GPIO.OUT)
    # GPIO SPI pin
    GPIO.setup("P8_9", GPIO.IN)
    # GPIO.setup(opened, GPIO.IN)
    # GPIO.setup(closed, GPIO.IN)
    # spi setup
    return


# the green LED blinks while waiting for confirmation
def green_led_blink():
    while True:
        GPIO.output("P8_10", GPIO.HIGH)
        time.sleep(0.5)
        GPIO.output("P8_10", GPIO.LOW)
        time.sleep(0.5)


# the red LED blinks while waiting for confirmation
def red_led_blink():
    while True:
        GPIO.output("P8_11", GPIO.HIGH)
        time.sleep(0.5)
        GPIO.output("P8_11", GPIO.LOW)
        time.sleep(0.5)


# the green LED turns on when a user has been accepted and box is opening
def green_led_on():
    GPIO.output("P8_10", GPIO.HIGH)
    return


# the red LED turns on when a user is closing the box
def red_led_on():
    GPIO.output("P8_11", GPIO.HIGH)
    return


# the yellow LED turns on when while loading
def yellow_led_on():
    GPIO.output("P8_14", GPIO.HIGH)
    return


# the green LED turns off
def green_led_off():
    GPIO.output("P8_10", GPIO.LOW)
    return


# the red LED turns off
def red_led_off():
    GPIO.output("P8_11", GPIO.LOW)
    return


# the yellow LED turns off
def yellow_led_off():
    GPIO.output("P8_14", GPIO.LOW)
    return


def spi_callback():
    return spi.xfer2(0)


# bbb sends and receives a value
def spi_response(resp):
    value = spi.xfer2(resp)
    return value[0]


# opening the box
def cmd_open_box():
    print("Box is closed.")
    print("Checking if there is a match...")
    yellow_led_on()
    red_led_off()
    green_led_off()
    # where it is checked, picture is taken and sent for confirmation of a match
    if take_picture_and_recognize():
        print("Match found. Opening box!")
        # send command to open the box
        spi.xfer2([openBox])
        time.sleep(0.560)
        #if opened == check_box_status():
        print("Box is open!")
        yellow_led_off()
        green_led_on()
    else:
        print("No match was found. Cannot open box.")
        green_led_off()
        yellow_led_off()
        red_led_on()
        send_alert("/var/lib/cloud9/theBox/unknown.jpg",0)
    time.sleep(0.1)


# closing the box
def cmd_close_box():
    print("Box is opened")
    print("Waiting to close box...")
    yellow_led_on()
    red_led_off()
    green_led_off()
    # send command to close the box
    spi.xfer2([closeBox])
    time.sleep(0.560)
    if closed == check_box_status():
        print("Closing box!")
        yellow_led_off()
        red_led_on()
    else:
        print("Error. Cannot close box")
        red_led_off()
    time.sleep(0.1)


# execute
def bbb_execute():
    bbb_init()
    red_led_on()
    while True:
        cmd_get_status()
        # if the button is pressed
        if GPIO.input("P8_12"):
            # if the state of the box is closed, then we check for a match to be able to open the box
            if closed == check_box_status():
                cmd_open_box()
            elif opened == check_box_status():
                cmd_close_box()
            else:
                print("Error. Box is neither opened nor closed")
        time.sleep(0.01)


# getting the status of the box
def cmd_get_status():
    if not GPIO.input("P8_9"):
        spi.xfer2([statusGPIO])
        if alertStatus == spi_response([0]):
            # we send an alert to user via email
            send_alert("/var/lib/cloud9/theBox/unknown.jpg",1)
            print("Sending email alert...")


def check_box_status():
    one_value = spi.xfer2([163])
    time.sleep(0.05)
    value = spi.xfer2([0])
    if value[0] == 0xFA:
        print("0xFA received...")
    else:
        print(one_value)
        print(value)
        sys.exit("Error with spi communication")
    # we check if GPIO is low
    time.sleep(0.56)
    if not GPIO.input("P8_9"):
        spi.xfer2([statusGPIO])
        time.sleep(0.05)
        resp = spi.xfer2([0])
        return resp[0]


# run program
bbb_execute()
