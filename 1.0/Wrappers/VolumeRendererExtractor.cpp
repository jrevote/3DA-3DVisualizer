/***********************************************************************
VolumeRendererExtractor - Wrapper class to map from the abstract
visualization algorithm interface to a templatized volume renderer
implementation.
Copyright (c) 2005-2007 Oliver Kreylos

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

#define VISUALIZATION_WRAPPERS_VOLUMERENDEREREXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>

#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/VolumeRenderer.h>

#include <Wrappers/VolumeRendererExtractor.h>

namespace Visualization {

namespace Wrappers {

/****************************************
Methods of class VolumeRendererExtractor:
****************************************/

template <class DataSetWrapperParam>
inline
const typename VolumeRendererExtractor<DataSetWrapperParam>::DS*
VolumeRendererExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("VolumeRendererExtractor::VolumeRendererExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename VolumeRendererExtractor<DataSetWrapperParam>::SE&
VolumeRendererExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("VolumeRendererExtractor::VolumeRendererExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
VolumeRendererExtractor<DataSetWrapperParam>::VolumeRendererExtractor(
	const GLColorMap* sColorMap,
	const Visualization::Abstract::DataSet* sDataSet,
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor,
	 Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sPipe),
	 colorMap(sColorMap),
	 ds(getDs(sDataSet)),
	 se(getSe(sScalarExtractor))
	{
	}

template <class DataSetWrapperParam>
inline
VolumeRendererExtractor<DataSetWrapperParam>::~VolumeRendererExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
bool
VolumeRendererExtractor<DataSetWrapperParam>::hasGlobalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
VolumeRendererExtractor<DataSetWrapperParam>::createElement(
	void)
	{
	/* Create a new volume renderer visualization element: */
	VolumeRenderer* result=new VolumeRenderer(ds,se,colorMap,getPipe());
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
VolumeRendererExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("VolumeRendererExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Create a new volume renderer visualization element: */
	return new VolumeRenderer(ds,se,colorMap,getPipe());
	}

template <class DataSetWrapperParam>
inline
bool
VolumeRendererExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("VolumeRendererExtractor::continueSlaveElement: Cannot be called on master node");
	
	/* It's really a dummy function; startSlaveElement() does all the work: */
	return true;
	}

}

}
