# Synchronizing Sound between wireless nodes

## General

Since we want to have multiple nodes playing music at the same time we need a mechanism to synchronize clocks among the raspberry pis. For this purpose we have a 
couple ideas which are described in the Write-Up. 

We tested different amounts of audio delay and came to the conclusion that all delays below 40ms should be fine for our purpose. Below this value, the delay is not 
perceived as an echo but it does degrade the quality. In the "sound delay example files" directory there are some audio files to test what different delays sound like.

We first attempted to synchronize music using microphones and speakers but when that didn't work we moved on to sound synchronization using Bluetooth which worked perfectly.


# Wiring
To see the success (or failure) of the testing scripts you need to wire LED's to WiringPi pin 0 which is GPIO17 on the Raspberry Pi 3/4. The test scripts are setup to light up the LED's at exactly the same time if clock synchronization was successful.
