#!/bin/sh

sudo /sbin/service VRDeviceDaemon stop
sudo /sbin/service VRDeviceDaemon start
sleep 20

/usr/local/packages/3DVisualizer-1.7/bin/3DVisualizer -class ImageStack ASKAPsim_Spectralline_Apr2010_fullfieldsubset.asc -rootSection dante

sudo /sbin/service VRDeviceDaemon stop
