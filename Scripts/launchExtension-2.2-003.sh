#!/bin/sh

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
sudo /sbin/service VRDeviceDaemon-2.2-003 start
sleep 20

/usr/local/packages/3DVisualizer/1.9/bin/3DVisualizer \
-class UnderworldHDF5File \
-mesh Mesh.linearMesh.00002.h5 \
-scalar VelocitySquaredField.00002.h5 \
-vector VelocityField.00002.h5 \
-rootSection dante

sudo /sbin/service VRDeviceDaemon-2.2-003 stop
