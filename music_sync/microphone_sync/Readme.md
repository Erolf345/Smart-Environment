# Microphone sync


For an in-depth explanation of these scripts please check the write-up.


## Instructions
To run these scripts the pi's need to be connected to the same network.

1. Run ./Pi_with_microphone *arbitrary_port* which will then calibrate the sound sensor (be very quiet) and then open a socket for Pi A to connect to on the given port.
2. Run ./Pi_with_speaker *hostname_of_B* *port_of_B* which will connect to Pi, A and proceed with the synchronization.
3. Pi_with_speaker will play a tone and a couple seconds after, the LED's will light up at (almost) the same time.
