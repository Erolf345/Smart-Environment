# import the necessary packages
from imutils.video import FileVideoStream
from imutils.video import FPS
import numpy as np
import argparse
import imutils
import time
import cv2

# start the file video stream thread and allow the buffer to start
fvs = FileVideoStream("movement3.mp4").start()
time.sleep(1.0)

# initialize variables
firstFrame = None
count = 0
distance_window = [800] * 20

# loop over frames from the video file stream
while fvs.more():
    # we use a counter to display frame info only every n times
    count = count + 1

    try:
        # grab the frame from the threaded video file stream,
        # resize it, and convert it to grayscale
        frame = fvs.read()
        frame = imutils.resize(frame, width=450)
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        gray = cv2.GaussianBlur(frame, (21, 21), 0)
        
        # if the first frame is None, initialize it
        # we use the first frame as a refernce for background substraction
        if firstFrame is None:
            firstFrame = gray
            continue
        
        # compute the absolute difference between the current frame and first frame
        frameDelta = cv2.absdiff(firstFrame, gray)
        thresh = cv2.threshold(frameDelta, 80, 255, cv2.THRESH_BINARY)[1]
        
        # extend the thresholded image to fill in holes, then find contours
        # on thresholded image
        thresh = cv2.dilate(thresh, None, iterations=2)
        cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
                            cv2.CHAIN_APPROX_SIMPLE)
        cnts = imutils.grab_contours(cnts)
        
        # because of shadow and camera movement, it can happen that we
        # detect more than one "motion" in a frame. We assume that the
        # biggest motion (e.g. contour) belongs to our person
        contour_size = 0
        selected_contour = None
        for c in cnts:
            current = cv2.contourArea(c)
            if current > contour_size:
                countour_size = current
                selected_contour = c
        
        # compute the bounding box for the contour, get the position and
        # dimensions and compute the distance (or draw a line)
        (x, y, w, h) = cv2.boundingRect(selected_contour)
        h_half = int(h*0.5)
        w_half = int(w*0.5)
        
        #cv2.line(frame, (0,0), (x+w_half, y+h_half), (0, 255, 0), 1)
        current_distance = np.round(np.sqrt(x*x + y*y), 2)
        if (current_distance == 0) or (current_distance is None): current_distance = 800
        distance_window.append(current_distance)
        distance_window.pop(0)
        
        avg_distance = sum(distance_window) / len(distance_window)
        # draw the text and timestamp on the frame
        cv2.putText(frame, "Distance to speaker: {}".format(avg_distance), (10, 20),
            cv2.FONT_HERSHEY_PLAIN, 1, (0, 0, 255), 1)
        
        # every 20 frames we print the avg_distance
        if count % 20 == 0:
            #cv2.imshow("Security Feed", frame)
            print(avg_distance)
        
    except AttributeError:
        break
    
    # keyboard interrupt: press q
    key = cv2.waitKey(1)
    if key == ord("q"):
        break

cv2.destroyAllWindows()
fvs.stop()


