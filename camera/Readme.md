# Camera

Setup without any issues following the official description of raspberry pi:

https://projects.raspberrypi.org/en/projects/getting-started-with-picamera/2

## Materials
 - Raspberry
 - Raspberry Pi Camera V2.1
 
## Result

One of the three tested cameras doesn't seem to work.

## Testable Project

For a first sensor-actuator test, setup camera as well as PIR sensor and run code ``camera_sensor.py`` on pi. Now every time a motion is detected, the camera will take an image.

## Motion Detector
run ```python /motion_detector.py --video /movement.MOV ``` from terminal and replace movement.MOV with path to any exemplary video file.

Requirements:
- opencv: Install with ```pip install opencv-python-headless```
- argparse: ```pip install argparse```
- datetime: ```pip install datetime```
- imutils: ```pip install imutils```

In line 48 (```thresh = cv2.threshold(frameDelta, 100, 255, cv2.THRESH_BINARY)[1]```) you can change the threshold (currently 100) to detect motion more easily or harder by de- or increasing it.

### Sources

https://www.pyimagesearch.com/2015/05/25/basic-motion-detection-and-tracking-with-python-and-opencv/
