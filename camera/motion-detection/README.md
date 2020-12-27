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
