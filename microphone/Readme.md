# About
This directory collects information about the microphones/sound sensors we are working with. There are 2. One has simple analog response to sound, so we need an additional ADC to connect it to the raspberry. The second one is I²S so we dont need any more hardware but we do need to install a kernel module for support.s

# Microphone 1
![alt text](pinout.webp "Logo Title Text 1")
## Materials
 - Raspberry
 - sound sensor (Sunfounder)
 - ADC (PCF8591)

## Results
Works

# Microphone 2

Since it is a I²S Microphone, we need to install a kernel module. The simplest installation is described here:

https://learn.adafruit.com/adafruit-i2s-mems-microphone-breakout/raspberry-pi-wiring-test

## Sources
https://www.sunfounder.com/learn/lesson-19-sound-sensor-sensor-kit-v2-0-for-b-plus.html
 
## Alternative Device
https://cdn-reichelt.de/documents/datenblatt/A300/DEBO_MEMS_MIC_DB_EN.pdf
