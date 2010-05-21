/***********************************************************************
CloudFile - Example class demonstrating how to write new modules for
3D Visualizer. Reads single-valued data in Cartesian coordinates from
simple ASCII files.
Copyright (c) 2008 Oliver Kreylos

This file is part of the 3D Data Visualizer (Visualizer).

The 3D Data Visualizer is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 2 of the License, or (at
your option) any later version.

The 3D Data Visualizer is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with the 3D Data Visualizer; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdio.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/CloudFile.h>

namespace Visualization {

namespace Concrete {

/******************************
Methods of class CloudFile:
******************************/

/***********************************************************************
Constructor for CloudFile class. Contains no code except the
definition of its own name.
***********************************************************************/

CloudFile::CloudFile(void)
	:BaseModule("CloudFile")
	{
	}

/***********************************************************************
Method to load a data set from a file, given a particular command line.
This method defines the format of the data files read by this module
class and has to be written.
***********************************************************************/

Visualization::Abstract::DataSet* CloudFile::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Create the result data set: */
	DataSet* result = new DataSet;
	
	/* Open the input file: */
	FILE* file=fopen(args[0].c_str(), "rt"); // args[0] is the first module command line parameter
	
	/* Read the input file's header: */
	DS::Index numVertices; // DS::Index is a helper type containing three integers NI, NJ, NK
	if(fscanf(file, "%d %d %d", &numVertices[0], &numVertices[1], &numVertices[2]));
	   {
	   /* Define the result data set's grid layout: */
	   DS& dataSet = result->getDs(); // Get the internal data representation from the result data set
	   dataSet.setGrid(numVertices); // Set the data set's number of vertices
	   dataSet.addSlice(); // Add a single scalar variable to the data set's grid
	
	   /* Define the result data set's variables as they are selected in 3D Visualizer's menus: */
	   DataValue& dataValue = result->getDataValue(); // Get the internal representations of the data set's value space
	   dataValue.initialize(&dataSet); // Initialize the value space for the data set
	   dataValue.setScalarVariableName(0, "Fraction"); // Set the name of the first scalar variable
	
	   /* Read all vertex positions and fraction values: */
	   DS::Index index; // Index counting variable containing three integers I, J, K
	   for(index[0] = 0; index[0] < numVertices[0]; ++index[0]) // I varies most slowly
		   for(index[1] = 0; index[1] < numVertices[1]; ++index[1])
		   	for(index[2] = 0; index[2] < numVertices[2]; ++index[2]) // K varies fastest
		   		{
		   		/* Read the vertex position and fraction value: */
		   		double pos[3], fraction;
		   		if(fscanf(file, "%lf %lf %lf %lf", &pos[0], &pos[1], &pos[2], &fraction))
		   	      {
                  //std::cout<<pos[0]<<","<<pos[1]<<","<<pos[2]<<","<<fraction<<"\n"<<std::flush;	
   
   				   /* Store the position and value in the data set: */
   				   dataSet.getVertexPosition(index) = DS::Point(pos); // Store the vertex' position
   				   dataSet.getVertexValue(0,index) = DS::ValueScalar(fraction); // Store the vertex' fraction
                  }
   				}
   	
   	/* Close the input file: */
   	fclose(file);
   	
   	/* Finalize the data set's grid structure (required): */
   	dataSet.finalizeGrid();
   	
   	/* Return the result data set: */
   	return result;
   	}
   }
}

}

/***********************************************************************
Plug-in interface functions. These allow loading dynamically loading
modules into 3D Visualizer at run-time, and do not have to be changed
except for the name of the generated module class.
***********************************************************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
   Visualization::Concrete::CloudFile* module = new Visualization::Concrete::CloudFile();

	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
