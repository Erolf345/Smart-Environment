## Structure

1. Folder raspi-camera-stream: Access camera module from Raspberry Pi and continuously check for motion in video live stream
2. ```motion_detector.py``` and ```videostream.py``` are tested to work with pre-recorded video file ```movement3.mp4``` 

## Prerequisites 

I closely followed these tutorials from pyimagesearch:

1. [Motion Detection and Tracking with Python and OpenCV on your PC](https://www.pyimagesearch.com/2015/05/25/basic-motion-detection-and-tracking-with-python-and-opencv/)
2. [Install OpenCV4 on Raspberry Pi](https://www.pyimagesearch.com/2019/09/16/install-opencv-4-on-raspberry-pi-4-and-raspbian-buster/)
3. [Accessing the Raspberry Pi Camera with OpenCV and Python](https://www.pyimagesearch.com/2015/03/30/accessing-the-raspberry-pi-camera-with-opencv-and-python/)
4. [Home Surviellance and Motion Detection with the Raspberry Pi and OpenCV](https://www.pyimagesearch.com/2015/06/01/home-surveillance-and-motion-detection-with-the-raspberry-pi-python-and-opencv/)

Finally, to make video processing faster on the Raspberry Pi, I used threading as described [here](http://www.pyimagesearch.com/2017/02/06/faster-video-file-fps-with-cv2-videocapture-and-opencv/)

## Important notes

- Contour Size and Threshold value in ```motion_detector.py``` are hyperparameters meant to control how easily motion is detected
- Sample video does not reflect our use case scenario as the person leaves the room in speaker's corner






