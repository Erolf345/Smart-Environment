from gpiozero import RGBLED
from colorzero import Color
from time import sleep

#define pins
led = RGBLED(green=10,red=9,blue=11,active_high=False) # common anode RBG LED!

led.on()
sleep(1)
led.off()

led.color = Color("yellow")
sleep(1)
led.color = Color("purple")
sleep(1)
led.toggle() #invert
sleep(1)
led.blink(on_time=1.5, off_time=0.5, fade_in_time=0.5, fade_out_time=0.5, on_color=(1, 0.3, 1), off_color=(0, 1, 0), n=10, background=True)

led.off()



