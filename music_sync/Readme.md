# Synchronizing Sound between wireless nodes

# General

Since we want to have multiple nodes playing musich at the same time we need a mechanism to synchronize clocks among the raspberry pis. For this purpose we have a 
couple ideas which we will describe here. 

We tested different amounts of audio delay and came to the conclusion that all delays below 40ms should be fine for our purpose. Below this value, the delay is not 
perceived as an echo but it does degrade the quality. In this directory there are some audio files to test what different delays sound like.

## synchronization using microphone

Pi A sends a clock time to Pi B at which it will play a sound. Pi B starts listening and waits until a certain volume level is exceeded. 
Pi B expects that this is the sound played by Pi B. It can now calculate the sound offset and play music/light a LED perfectly synchronized.

This process was coded in C to make sure that commands are executed as quickly as possible. Currently, the sounds are played by executing a shell command 
which probably creates an unnecessary large overhead. Currently, the workaround is to subtract 90ms from the clock time the other raspberry detects. 
Accuracy was measured by filming activation of LEDs with a slow motion camera at 240 fps. With this setup the LEDs light up within 1 frame of each other, 
which gives us an accuracy of 4ms.
