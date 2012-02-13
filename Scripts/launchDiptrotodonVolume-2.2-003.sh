#!/bin/sh

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
sudo /sbin/service VRDeviceDaemon-2.2-003 start
sleep 20

/usr/local/packages/3DVisualizer/1.9/bin/3DVisualizer -class ImageStack /usr/local/packages/3DVisualizer/Datasets/Diptrotodon/Diptrotodon.asc -rootSection dante

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
