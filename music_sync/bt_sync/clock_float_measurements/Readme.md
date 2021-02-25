# Clock drift measurements

# About
In this part we created a script for accurately measuring the bluetooth clock drift. Since GPIO pins have the smallest possible delay, we trigger one pin on Pi A in a certain interval and print out the corresponding bluetooth clock at each trigger. Pi B is connected to the Pin and predicts A's bluetooth clock each time it reads HIGH on the pin (IMPORTANT: read section wiring). When running this code for a long time the clock drift becomes more pronounced.

# Perequisites
see bt_sync

## Wiring
IMPORTANT: For this setup we did not have any success when trying to design wiring using pull up/down resistors. A single wrong trigger would break the entire script. We discovered a workaround hack accidentally that the pins do not float randomly when Ground of Pi A and Pi B are connected. In this scenario we can directly connect GPIO 17 of Pi A and B for the fastest possible communication

## Compilation and execution

Same as bt_sync