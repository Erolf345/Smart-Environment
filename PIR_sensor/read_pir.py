#!/usr/bin/python

import RPi.GPIO as GPIO
import time

GPIO.setmode(GPIO.BOARD)  # Set GPIO to pin numbering
pir = 18  # Assign pin 8 to PIR
GPIO.setup(pir, GPIO.IN)  # Setup GPIO pin PIR as input
print("Sensor initializing . . .")
time.sleep(2)  # Give sensor time to startup
print("Active")
print("Press Ctrl+c to end program")

try:
    while True:
        if GPIO.input(pir) == True:  # If PIR pin goes high, motion is detected
            print("Motion Detected!", end='\r')
        else:
            print("                  ", end='\r')
        time.sleep(0.1)

except KeyboardInterrupt:  # Ctrl+c
    pass  # Do nothing, continue to finally

finally:
    GPIO.cleanup()  # reset all GPIO
    print("Program ended")
