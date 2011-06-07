/***********************************************************************
Module - Wrapper class to combine templatized data set representations
and templatized algorithms into a polymorphic visualization module.
Copyright (c) 2006-2007 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_MODULE_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>

#include <Wrappers/DataSet.h>
#include <Wrappers/DataSetRenderer.h>
#include <Wrappers/SeededSliceExtractor.h>
#include <Wrappers/SeededIsosurfaceExtractor.h>
#include <Wrappers/VolumeRendererExtractor.h>
#include <Wrappers/ArrowRakeExtractor.h>
#include <Wrappers/StreamlineExtractor.h>
#include <Wrappers/MultiStreamlineExtractor.h>
// #include <Wrappers/StreamsurfaceExtractor.h>

#include <Wrappers/Module.h>

namespace Visualization {

namespace Wrappers {

/***********************
Methods of class Module:
***********************/

template <class DSParam,class DataValueParam>
inline
Visualization::Abstract::DataSetRenderer*
Module<DSParam,DataValueParam>::getRenderer(
	const Visualization::Abstract::DataSet* dataSet) const
	{
	return new DataSetRenderer(dataSet);
	}

template <class DSParam,class DataValueParam>
inline
int
Module<DSParam,DataValueParam>::getNumScalarAlgorithms(
	void) const
	{
	return 3;
	}

template <class DSParam,class DataValueParam>
inline
const char*
Module<DSParam,DataValueParam>::getScalarAlgorithmName(
	int scalarAlgorithmIndex) const
	{
	if(scalarAlgorithmIndex<0||scalarAlgorithmIndex>=3)
		Misc::throwStdErr("Module::getAlgorithmName: invalid algorithm index %d",scalarAlgorithmIndex);
	
	static const char* algorithmNames[3]=
		{
		"Seeded Slice","Seeded Isosurface","Volume Renderer"
		};
	
	return algorithmNames[scalarAlgorithmIndex];
	}

template <class DSParam,class DataValueParam>
inline
Visualization::Abstract::Algorithm*
Module<DSParam,DataValueParam>::getScalarAlgorithm(
	int scalarAlgorithmIndex,
	const GLColorMap* colorMap,
	const Visualization::Abstract::DataSet* dataSet,
	const Visualization::Abstract::ScalarExtractor* scalarExtractor,
	Comm::MulticastPipe* pipe) const
	{
	if(scalarAlgorithmIndex<0||scalarAlgorithmIndex>=3)
		Misc::throwStdErr("Module::getAlgorithm: invalid algorithm index %d",scalarAlgorithmIndex);
	
	Visualization::Abstract::Algorithm* result=0;
	switch(scalarAlgorithmIndex)
		{
		case 0:
			result=new SeededSliceExtractor(colorMap,dataSet,scalarExtractor,pipe);
			break;
		
		case 1:
			{
			SeededIsosurfaceExtractor* ise=new SeededIsosurfaceExtractor(colorMap,dataSet,scalarExtractor,pipe);
			ise->getIse().setExtractionMode(SeededIsosurfaceExtractor::ISE::SMOOTH);
			result=ise;
			break;
			}
		
		case 2:
			{
			VolumeRendererExtractor* vre=new VolumeRendererExtractor(colorMap,dataSet,scalarExtractor,pipe);
			result=vre;
			break;
			}
		}
	return result;
	}

template <class DSParam,class DataValueParam>
inline
int
Module<DSParam,DataValueParam>::getNumVectorAlgorithms(
	void) const
	{
	// return 4;
	return 3;
	}

template <class DSParam,class DataValueParam>
inline
const char*
Module<DSParam,DataValueParam>::getVectorAlgorithmName(
	int vectorAlgorithmIndex) const
	{
	// if(vectorAlgorithmIndex<0||vectorAlgorithmIndex>=4)
	if(vectorAlgorithmIndex<0||vectorAlgorithmIndex>=3)
		Misc::throwStdErr("Module::getVectorAlgorithmName: invalid algorithm index %d",vectorAlgorithmIndex);
	
	static const char* vectorAlgorithmNames[3]=
		{
		"Arrow Rake","Streamline","Streamline Bundle" // ,"Stream Surface"
		};
	
	return vectorAlgorithmNames[vectorAlgorithmIndex];
	}

template <class DSParam,class DataValueParam>
inline
Visualization::Abstract::Algorithm*
Module<DSParam,DataValueParam>::getVectorAlgorithm(
	int vectorAlgorithmIndex,
	const GLColorMap* colorMap,
	const Visualization::Abstract::DataSet* dataSet,
	const Visualization::Abstract::VectorExtractor* vectorExtractor,
	const Visualization::Abstract::ScalarExtractor* scalarExtractor,
	Comm::MulticastPipe* pipe) const
	{
	// if(vectorAlgorithmIndex<0||vectorAlgorithmIndex>=4)
	if(vectorAlgorithmIndex<0||vectorAlgorithmIndex>=3)
		Misc::throwStdErr("Module::getAlgorithm: invalid algorithm index %d",vectorAlgorithmIndex);
	
	Visualization::Abstract::Algorithm* result=0;
	switch(vectorAlgorithmIndex)
		{
		case 0:
			result=new ArrowRakeExtractor(colorMap,dataSet,vectorExtractor,scalarExtractor,pipe);
			break;
		
		case 1:
			result=new StreamlineExtractor(colorMap,dataSet,vectorExtractor,scalarExtractor,pipe);
			break;
		
		case 2:
			result=new MultiStreamlineExtractor(colorMap,dataSet,vectorExtractor,scalarExtractor,dataSet->calcAverageCellSize(),pipe);
			break;
		
		#if 0
		case 3:
			result=new StreamsurfaceExtractor(colorMap,dataSet,vectorExtractor,scalarExtractor,dataSet->calcAverageCellSize());
			break;
		#endif
		}
	return result;
	}

}

}
