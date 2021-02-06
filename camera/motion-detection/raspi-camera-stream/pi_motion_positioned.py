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
import sys

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
#print("[INFO] warming up...")
time.sleep(2.5)
avg = None
motionCounter = 0


resize_width=500

### Transforms a coordinate to the cameras relative position in the room
class PositionTransformer:
    
    def __init__(self,upleft,downright):
        self.l = upleft#[0,0]
        self.r = downright#[640,480]

        x_dist = self.r[0] - self.l[0]
        y_dist = self.r[1] - self.l[1]

        self.x_scale = abs(x_dist)/resize_width#camera.resolution[0]
        self.y_scale = abs(y_dist)/(camera.resolution[1] * resize_width/camera.resolution[0])


        if x_dist > 0:
            if y_dist > 0:
                self.pos = 0 #normal positioning
            else:
                self.pos = 1 # right turn 
        elif x_dist < 0:
            if y_dist > 0:
                self.pos = 3 # left turn
            else:
                self.pos = 2 #upside down
        
        #print(self.pos)

    def scale(self,coord):
        return [self.x_scale*coord[0], self.y_scale*coord[1]]

    def transform(self, coord):
        #assuming that all cams use same resolution
        # if not add scaling
        new_coord = coord
        
        #print(str(coord) + " before")
        #print(coord,"first")
        
        coord = self.scale(coord)
        #print(coord)
        if self.pos == 0:
            new_coord = [self.l[0] + coord[0], self.l[1]+coord[1]] 
        elif self.pos == 2: #upside down
            new_coord = [self.l[0] - coord[0], self.l[1] - coord[1]]
        elif self.pos == 1: # turned right
            new_coord = [self.l[0] + coord[1], self.l[1]-coord[0]]
        else: # turned left
            new_coord = [self.l[0] - coord[1], self.l[1] + coord[0]]

        #print(str(coord) + " after")
        return new_coord#self.scale(new_coord)

# pass arguments as python3 filenam.py xleft,yleft xright,yright
x_pos_cam = [float(sys.argv[1].split(",")[0]),float(sys.argv[1].split(",")[1])]
y_pos_cam = [float(sys.argv[2].split(",")[0]),float(sys.argv[2].split(",")[1])]#[1,1]
pos_trans = PositionTransformer(x_pos_cam,y_pos_cam)

# capture frames from the camera
for f in camera.capture_continuous(rawCapture, format="bgr", use_video_port=True):
	# grab the raw NumPy array representing the image
	frame = f.array
	# resize the frame, convert it to grayscale, and blur it
	frame = imutils.resize(frame, width=resize_width)
	gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
	gray = cv2.GaussianBlur(gray, (21, 21), 0)
	# if the average frame is None, initialize it
	if avg is None:
		#print("[INFO] starting background model...")
		avg = gray.copy().astype("float")
		rawCapture.truncate(0)
		continue
	# accumulate the weighted average between the current frame and
	# previous frames, (!! not needed in our scenario!!)
	# then compute the difference between the current frame and running average
	cv2.accumulateWeighted(gray, avg, 0.25)
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
	#compute motion's position based on cameras relative position 
	
	centroid = pos_trans.transform([x+w_half,y+h_half])
	#centroid = [x+w_half,y+h_half]
	
	if distance < 625: #print(distance)
		print(str(centroid[0])+","+str(centroid[1]))
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
