#!/usr/bin/env python

import sys
import re
import math

# Make sure that a filename is specified.
if len(sys.argv) < 3:
   print "You must supply a file name to convert and the" \
         "filename of the output file.\n" \
         "For example: ./convertModelToVTK.py NVP_model.01_04 SashaVTK.vts\n"
   sys.exit()

# Try open the file and perform conversion.
try:
   model_file = open(sys.argv[1])
except IOError:
   print 'Cannot open: ', sys.argv[1]
else:
   # Regular expressions for our file.
   headerwithlm = re.compile(r'^#Iteration\s+No.\s*(?P<iteration>\d+)\s*' 
                       'RMS\s+=\s+(?P<rms>\d+\.*\d*)\s*' 
                       'LM\s+=\s+(?P<lm>\d+\.*\d*)\s*$')
   headernolm = re.compile(r'^#Iteration\s+No.\s*(?P<iteration>\d+)\s*' 
                       'RMS\s+=\s+(?P<rms>\d+\.*\d*)\s*$') 
   block = re.compile(r'^\s*(?P<xblocks>\d+)\s+' \
                      '(?P<yblocks>\d+)\s+' \
                      '(?P<zblocks>\d+)\s+' \
                      '(?P<extra>\d+)$')
   
   # Some variables.
   xcounter, ycounter, zcounter, dcounter = 0, 0, 0, 0
   xblocks, yblocks, zblocks = 0, 0, 0
   xnodes, ynodes, znodes = 0, 0, 0
   total_blocks, total_nodes = 0, 0
   block_sizes = {'X':[0.0], 'Y':[0.0], 'Z':[0.0]}
   block_data = []
   read_blocks = True
   read_data = False
   
   try:
      vtk_file = open(sys.argv[2], "w")
   except IOError:
      print 'Cannon open: ', sys.argv[2]
   else:
   
      # Write VTK info to the output file.<?xml version="1.0"?>
      vtk_file.write('<?xml version=\"1.0\"?>\n'
                     '<VTKFile type=\"StructuredGrid\" version=\"0.1\"'
                     ' byte_order=\"BigEndian\">\n')
   
      # Loop through each line and extract block values/data.
      for line in model_file.readlines():
         headerwithlm_match = headerwithlm.match(line)
         headernolm_match = headernolm.match(line)
         block_match = block.match(line)
      
         # Check if it's the header line.
         if headerwithlm_match:
            print "Processing", sys.argv[1], "...."
            print "Iteration:", int(headerwithlm_match.group('iteration'))
            print "RMS:", float(headerwithlm_match.group('rms'))
            print "LM:", float(headerwithlm_match.group('lm'))
         elif headernolm_match:
            print "Processing", sys.argv[1], "...."
            print "Iteration:", int(headernolm_match.group('iteration'))
            print "RMS:", float(headernolm_match.group('rms'))
         
         # Check if it's the block sizes line.
         elif block_match:
            # Assign the cell dimension.
            xblocks = int(block_match.group('xblocks'))
            yblocks = int(block_match.group('yblocks'))
            zblocks = int(block_match.group('zblocks'))
         
            # Assign the nodal dimension.
            xnodes = xblocks + 1
            ynodes = yblocks + 1
            znodes = zblocks + 1
         
            # Assign total number of blocks and nodes.
            total_blocks = xblocks * yblocks * zblocks
            total_nodes = xnodes * ynodes * znodes
         
            # Just print the values.
            print "Cell Dimension:", xblocks, yblocks, zblocks
            print "Nodal Dimension:", xnodes, ynodes, znodes
         
         # Let's read the block sizes values and cell data.   
         else:
            # Check if we're still reading block sizes info.
            if read_blocks:
               # Split the line to get the block sizes.
               sizes = line.split()
               coord = 'X'
            
               if xcounter < xblocks:
                  xcounter += len(sizes)
                  coord = 'X'
               elif ycounter < yblocks:
                  ycounter += len(sizes)
                  coord = 'Y'
               elif zcounter < zblocks:
                  zcounter += len(sizes)
                  coord = 'Z'
              
               if read_blocks:   
                  for size in sizes: 
                     block_sizes[coord].append(float(size))
               
               if zcounter == zblocks:
                  print block_sizes['X']
                  print block_sizes['Y']
                  print block_sizes['Z']
                  read_blocks = False
                  read_data = True
                  
            # Check if we're reading the resistivity data.          
            elif read_data:
               if dcounter < total_blocks:
                  values = line.split()
                  for value in values:
                     block_data.append(math.log10(float(value)))
                     dcounter += 1
               else:
                  read_data = False    
   
      # Write the first section of the VTK output file.               
      vtk_file.write('<StructuredGrid WholeExtent=\"'
                     '0 ' + str(zblocks) +
                     ' 0 ' + str(yblocks) +
                     ' 0 ' + str(xblocks) + '\">\n' +
                     '<Piece Extent=\"'
                     '0 ' + str(zblocks) +
                     ' 0 ' + str(yblocks) +
                     ' 0 ' + str(xblocks) + '\">\n'
                     '<Points>\n'
                     '<DataArray Name=\"xyz\" NumberOfComponents=\"3\"'
                     ' type=\"Float64\" format=\"ascii\">\n')
  
      posx, posy, posz = 0, 0, 0
      # Generate the nodes position.
      for x_i in range(xnodes):
         posy = 0
         posz += float(block_sizes['X'][x_i])
         for y_i in range(ynodes):
            posx = 0
            posy += float(block_sizes['Y'][y_i])
            for z_i in range(znodes):
               posx += float(block_sizes['Z'][z_i])
               # Write the node coordinates to the file.
               vtk_file.write(str("%.6f" % posx) + " " +
                              str("%.6f" % posy) + " " +
                              str("%.6f" % posz) + "\n")
   
      # Write the ending tags for the points.         
      vtk_file.write('</DataArray>\n'
                     '</Points>\n' 
                     '<CellData>\n'
                     '<DataArray Name=\"Resistivity\" '
                     'NumberOfComponents=\"1\" '
                     'type=\"Float64\" format=\"ascii\">\n')
   
      # Write the resistivity cell values.
      for r_i in range(0, len(block_data)):
         vtk_file.write(str("%.6f" % block_data[r_i]) + "\n")
         
      # Write ending tags to finish the VTK file.        
      vtk_file.write('</DataArray>\n')
      vtk_file.write('</CellData>\n')
      vtk_file.write('</Piece>\n')
      vtk_file.write('</StructuredGrid>\n')
      vtk_file.write('</VTKFile>\n')
   
      # Close the file.
      vtk_file.close()
      model_file.close()
