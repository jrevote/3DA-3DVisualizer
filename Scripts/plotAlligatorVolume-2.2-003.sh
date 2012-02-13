#!/bin/sh

sudo /sbin/service VRDeviceDaemon stop
sudo /sbin/service VRDeviceDaemon start
sleep 20

/usr/local/packages/3DVisualizer/1.7/bin/3DVisualizer -class ImageStack Alligator.asc -rootSection dante

sudo /sbin/service VRDeviceDaemon stop
