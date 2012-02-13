#!/bin/sh

sudo /sbin/service VRDeviceDaemon-1.0-068 stop
sudo /sbin/service VRDeviceDaemon-1.0-068 start
sleep 20

/usr/local/packages/3DVisualizer/1.7/bin/3DVisualizer -class ImageStack /usr/local/packages/3DVisualizer/Datasets/Mouse/Mouse.asc -rootSection dante

sudo /sbin/service VRDeviceDaemon-1.0-068 stop
