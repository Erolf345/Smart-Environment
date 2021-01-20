# import the necessary packages
from picamera.array import PiRGBArray
from picamera import PiCamera
import numpy as np
import argparse
import warnings
import imutils
import json
import time
import cv2

# construct the argument parser and parse the arguments
#ap = argparse.ArgumentParser()
#ap.add_argument("-c", "--conf", required=True,
#	help="path to the JSON configuration file")
#args = vars(ap.parse_args())

warnings.filterwarnings("ignore")
#conf = json.load(open(args["conf"]))
show_video = True

# initialize the camera and grab a reference to the raw camera capture
camera = PiCamera()
camera.resolution = tuple([640,480])
camera.framerate = 10
rawCapture = PiRGBArray(camera, size=tuple([640, 480]))

# allow the camera to warmup, then initialize the average frame,
# and frame motion counter
print("[INFO] warming up...")
time.sleep(2.5)
avg = None
motionCounter = 0

# capture frames from the camera
for f in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
	# grab the raw NumPy array representing the image
	frame = f.array
	# resize the frame, convert it to grayscale, and blur it
	frame = imutils.resize(frame, width=500)
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	gray = cv2.GaussianBlur(gray, (21, 21), 0)
	# if the average frame is None, initialize it
	if avg is None:
		print("[INFO] starting background model...")
		avg = gray.copy().astype("float")
		rawCapture.truncate(0)
		continue
	# accumulate the weighted average between the current frame and
	# previous frames, (!! not needed in our scenario!!)
	# then compute the difference between the current frame and running average
	#cv2.accumulateWeighted(gray, avg, 0.25)
	frameDelta = cv2.absdiff(gray, cv2.convertScaleAbs(avg))
	
	# threshold the delta image, dilate the thresholded image to fill
	# in holes, then find contours on thresholded image
	thresh = cv2.threshold(frameDelta, 40, 255,
		cv2.THRESH_BINARY)[1]
	thresh = cv2.dilate(thresh, None, iterations=2)
	cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
		cv2.CHAIN_APPROX_SIMPLE)
	cnts = imutils.grab_contours(cnts)
	
	# iterate over all contour and select largest that is above min_area
	size_max = 0
	largest_relevant_cont = None
	for c in cnts:
		size_current_cont = cv2.contourArea(c)
		if (size_current_cont >= size_max) & (size_current_cont >= 1000):
				size_max = size_current_cont
				largest_relevant_cont = c
	
	# compute bounding rectangle around our selected contour
	(x, y, w, h) = cv2.boundingRect(largest_relevant_cont)
	h_half = int(h*.5)
	w_half = int(w*.5)
	
	cv2.line(frame, (0,0), (x+w_half, y+h_half), (0, 255, 0), 2)
	cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
	distance = np.round(np.sqrt((x+w_half)**2 + (y+h_half)**2))
	
	if distance == 0: distance = 625
	
	if distance < 625: print(distance)
	#cv2.putText(frame, "Distance: {}".format(distance), (10, 20), cv2.FONT_HERSHEY_PLAIN, 2, (0, 0, 255), 1)
	# check to see if the frames should be displayed to screen
	if show_video:
		# display the security feed
		#cv2.imshow("Video", frame)
		#cv2.imshow("Threshold", thresh)
		#cv2.imshow("Difference", frameDelta)
		key = cv2.waitKey(1) & 0xFF
		# if the `q` key is pressed, break from the lop
		if key == ord("q"):
			break
	# clear the stream in preparation for the next frame
	rawCapture.truncate(0)
        