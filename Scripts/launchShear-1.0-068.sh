#!/bin/sh

/usr/local/packages/3DVisualizer/1.7/bin/3DVisualizer \
-class UnderworldHDF5File \
-mesh Mesh.linearMesh.00125.h5 \
-scalar VelocityGradientsField.00125.h5 StrainRateInvariantField.00125.h5 \
-vector VelocityField.00125.h5 \
-rootSection dante
