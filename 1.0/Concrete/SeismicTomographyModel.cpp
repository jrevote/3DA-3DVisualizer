/***********************************************************************
SeismicTomographyModel - Class to visualize results of seismic
tomographic analyses in Mercator grid format.
Copyright (c) 2007 Oliver Kreylos

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

#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Math/Constants.h>

#include <Concrete/EarthDataSet.h>

#include <Concrete/SeismicTomographyModel.h>

namespace Visualization {

namespace Concrete {

/**************************************
Methods of class SeismicTomographyModel:
**************************************/

SeismicTomographyModel::SeismicTomographyModel(void)
	:BaseModule("SeismicTomographyModel")
	{
	}

Visualization::Abstract::DataSet* SeismicTomographyModel::load(const std::vector<std::string>& args) const
	{
	/* Parse the module command line: */
	bool haveNumVertices=false;
	DS::Index numVertices;
	const char* dataFileName=0;
	int column[4]={0,1,2,3}; // Default column order is latitude, longitude, depth, value
	bool cellCentered=false;
	int order[3]={0,1,2}; // Default data order is lat varies fastest, depth varies most slowly
	bool invert=false; // Flag whether to invert the order of the innermost loop
	for(unsigned int i=0;i<args.size();++i)
		{
		if(args[i][0]=='-')
			{
			if(strcasecmp(args[i].c_str()+1,"size")==0) // Size is given in lat, long, depth order
				{
				int j;
				for(j=0;j<3&&i<args.size()-1;++j)
					{
					++i;
					numVertices[j]=atoi(args[i].c_str());
					}
				haveNumVertices=j==3;
				}
			else if(strcasecmp(args[i].c_str()+1,"cell")==0)
				{
				/* Data is cell-centered: */
				cellCentered=true;
				}
			else if(strcasecmp(args[i].c_str()+1,"invert")==0)
				{
				/* Data order must be inverted: */
				invert=true;
				}
			else if(strcasecmp(args[i].c_str()+1,"column")==0) // Column order is given in lat, long, depth, value order
				{
				int j;
				for(j=0;j<4&&i<args.size()-1;++j)
					{
					++i;
					column[j]=atoi(args[i].c_str());
					}
				if(j<4)
					Misc::throwStdErr("SeismicTomographyModel::load: Too few components in -column option");
				int columnMask=0x0;
				for(int j=0;j<4;++j)
					columnMask|=0x1<<column[j];
				if(columnMask!=0xf)
					Misc::throwStdErr("SeismicTomographyModel::load: -column option does not define a permutation");
				}
			else if(strcasecmp(args[i].c_str()+1,"order")==0) // Variation order is given in lat, long, depth order where 0 varies fastest
				{
				int j;
				for(j=0;j<3&&i<args.size()-1;++j)
					{
					++i;
					order[j]=atoi(args[i].c_str());
					}
				if(j<3)
					Misc::throwStdErr("SeismicTomographyModel::load: Too few components in -order option");
				int orderMask=0x0;
				for(int j=0;j<3;++j)
					orderMask|=0x1<<order[j];
				if(orderMask!=0x7)
					Misc::throwStdErr("SeismicTomographyModel::load: -order option does not define a permutation");
				}
			}
		else
			dataFileName=args[i].c_str();
		}
	if(!haveNumVertices)
		Misc::throwStdErr("SeismicTomographyModel::load: Missing data set size");
	if(dataFileName==0)
		Misc::throwStdErr("SeismicTomographyModel::load: Missing data set file name");
	
	/* Open the input wave velocity file: */
	Misc::File vFile(dataFileName,"rt");
	
	/* Data size is depth, longitude, latitude in C memory order (latitude varies fastest) */
	DS::Index gridSize=numVertices;
	if(cellCentered)
		++gridSize[1]; // Replicate meridian to stich grid
	
	/* Create the data set: */
	EarthDataSet<DataSet>* result=new EarthDataSet<DataSet>(args);
	result->getDs().setGrids(1);
	result->getDs().setGridData(0,gridSize); // Make extra room to stitch at 0 meridian
	
	/* Set the data value's name: */
	result->getDataValue().setScalarVariableName("Differential Wave Velocity");
	
	/* Constant parameters for geoid formula: */
	const double a=6378.14e3; // Equatorial radius in m
	const double f=1.0/298.247; // Geoid flattening factor
	const double scaleFactor=1.0e-3;
	
	/* Compute the mapping from file order to C memory order: */
	int am[3];
	for(int i=0;i<3;++i)
		{
		for(int j=0;j<3;++j)
			if(order[j]==2-i)
				am[i]=j;
		}
	
	/* Read all grid points from the input file: */
	DS::Array& vertices=result->getDs().getGrid(0).getVertices();
	DS::Index index;
	for(index[am[0]]=0;index[am[0]]<numVertices[am[0]];++index[am[0]])
		{
		for(index[am[1]]=0;index[am[1]]<numVertices[am[1]];++index[am[1]])
			{
			if(invert)
				{
				for(index[am[2]]=numVertices[am[2]]-1;index[am[2]]>=0;--index[am[2]])
					{
					/* Read the next grid point from the file: */
					char line[80];
					vFile.gets(line,sizeof(line));
					double col[4];
					sscanf(line,"%lf %lf %lf %lf",&col[0],&col[1],&col[2],&col[3]);
					double lat=col[column[0]];
					double lng=col[column[1]];
					double depth=col[column[2]];
					float value=float(col[column[3]]);
					
					/* Convert geoid coordinates to Cartesian coordinates: */
					DS::GridVertex& vertex=vertices(index);
					lat=Math::rad(lat);
					lng=Math::rad(lng);
					double s0=Math::sin(lat);
					double c0=Math::cos(lat);
					double r=(a*(1.0-f*Math::sqr(s0))-depth*1000.0)*scaleFactor;
					double xy=r*c0;
					double s1=Math::sin(lng);
					double c1=Math::cos(lng);
					vertex.pos[0]=float(xy*c1);
					vertex.pos[1]=float(xy*s1);
					vertex.pos[2]=float(r*s0);
					
					/* Store the velocity value: */
					vertex.value=value;
					}
				}
			else
				{
				for(index[am[2]]=0;index[am[2]]<numVertices[am[2]];++index[am[2]])
					{
					/* Read the next grid point from the file: */
					char line[80];
					vFile.gets(line,sizeof(line));
					double col[4];
					sscanf(line,"%lf %lf %lf %lf",&col[0],&col[1],&col[2],&col[3]);
					double lat=col[column[0]];
					double lng=col[column[1]];
					double depth=col[column[2]];
					float value=float(col[column[3]]);
					
					/* Convert geoid coordinates to Cartesian coordinates: */
					DS::GridVertex& vertex=vertices(index);
					lat=Math::rad(lat);
					lng=Math::rad(lng);
					double s0=Math::sin(lat);
					double c0=Math::cos(lat);
					double r=(a*(1.0-f*Math::sqr(s0))-depth*1000.0)*scaleFactor;
					double xy=r*c0;
					double s1=Math::sin(lng);
					double c1=Math::cos(lng);
					vertex.pos[0]=float(xy*c1);
					vertex.pos[1]=float(xy*s1);
					vertex.pos[2]=float(r*s0);
					
					/* Store the velocity value: */
					vertex.value=value;
					}
				}
			}
		}
	
	if(cellCentered)
		{
		/* Stitch the grid across the longitude boundaries: */
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				vertices(index[0],numVertices[1],index[2])=vertices(index[0],0,index[2]);
		}
	
	/* Finalize the grid structure: */
	result->getDs().finalizeGrid();
	
	return result;
	}

Visualization::Abstract::DataSetRenderer* SeismicTomographyModel::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
	{
	return new EarthDataSetRenderer<DataSet,DataSetRenderer>(dataSet);
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::SeismicTomographyModel* module=new Visualization::Concrete::SeismicTomographyModel();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
