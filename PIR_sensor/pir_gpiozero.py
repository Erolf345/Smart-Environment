from gpiozero import MotionSensor

pir = MotionSensor(4)
pir.wait_for_motion()
print("Motion detected!")


"""
#with LED

from gpiozero import MotionSensor, LED
from signal import pause

pir = MotionSensor(4)
led = LED(17)

pir.when_motion = led.on
pir.when_no_motion = led.off

pause()
"""
