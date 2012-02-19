#!/usr/bin/python

'''
Created on Jan 29, 2012

@author: jsnavely
'''

import cv
import sys
from math import sin, cos, sqrt, pi, degrees
from  numpy import median


def wait():
    while True:
        k = cv.WaitKey(0) % 0x100
        if k == 27:
            break

fname =  sys.argv[1]
original= cv.LoadImage(fname)

img = cv.CreateImage( cv.GetSize(original), cv.IPL_DEPTH_8U, 1)
cv.CvtColor(original,img, cv.CV_BGR2GRAY)

cv.AdaptiveThreshold(img, img, 255.0, cv.CV_THRESH_BINARY, cv.CV_ADAPTIVE_THRESH_MEAN_C,9)


# down-scale and upscale the image to filter out the noise
pyr = cv.CreateImage((img.width/2, img.height/2),  cv.IPL_DEPTH_8U, 1)

cv.PyrDown(img, pyr, 7)
cv.PyrUp(pyr, img, 7)

cv.Smooth(img, img, cv.CV_MEDIAN, 1, 5 )

#cv.Dilate(img,img,None,1)
#cv.Erode(img,img,None,1)

cv.AdaptiveThreshold(img, img, 255.0, cv.CV_THRESH_BINARY, cv.CV_ADAPTIVE_THRESH_MEAN_C,9)

showme= cv.CloneImage(img)
size = cv.GetSize(img)

cv.ShowImage("Show Me", img )

cv.SaveImage("imgfix.jpg",img)

wait()
