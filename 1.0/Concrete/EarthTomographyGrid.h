/***********************************************************************
EarthTomographyGrid - Class to visualize results of seismic tomographic
analyses in Mercator grid format.
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

#ifndef VISUALIZATION_CONCRETE_EARTHTOMOGRAPHYGRID_INCLUDED
#define VISUALIZATION_CONCRETE_EARTHTOMOGRAPHYGRID_INCLUDED

#include <Wrappers/MultiCurvilinearIncludes.h>
#include <Wrappers/SingleScalarValue.h>

#include <Wrappers/Module.h>

namespace Visualization {

namespace Concrete {

namespace {

/* Basic type declarations: */
typedef float Scalar; // Scalar type of data set domain
typedef float VScalar; // Scalar type of data set value
typedef float Value; // Memory representation of data set value
typedef Visualization::Templatized::MultiCurvilinear<Scalar,3,Value> DS; // Templatized data set type
typedef Visualization::Wrappers::SingleScalarValue<DS,VScalar> DataValue; // Type of data value descriptor
typedef Visualization::Wrappers::Module<DS,DataValue> BaseModule; // Module base class type

}

class EarthTomographyGrid:public BaseModule
	{
	/* Constructors and destructors: */
	public:
	EarthTomographyGrid(void); // Default constructor
	
	/* Methods: */
	virtual Visualization::Abstract::DataSet* load(const std::vector<std::string>& args) const;
	virtual Visualization::Abstract::DataSetRenderer* getRenderer(const Visualization::Abstract::DataSet* dataSet) const;
	};

}

}

#endif
