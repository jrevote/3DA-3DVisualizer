/***********************************************************************
StructuredGocadFile - Class defining lowest-common-denominator ASCII
file format for curvilinear grids in Cartesian or spherical coordinates.
Vertex positions and attributes are stored in separate files.
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

#ifndef VISUALIZATION_CONCRETE_STRUCTRUREDGRIDASCII_INCLUDED
#define VISUALIZATION_CONCRETE_STRUCTRUREDGRIDASCII_INCLUDED

#include <Wrappers/GocadCurvilinearIncludes.h>
#include <Wrappers/SlicedScalarVectorDataValue.h>

#include <Wrappers/Module.h>

namespace Visualization {

namespace Concrete {

namespace {

/* Basic type declarations: */
typedef float Scalar; // Scalar type of data set domain
typedef float VScalar; // Scalar type of data set value
typedef Visualization::Templatized::SlicedCurvilinear<Scalar,3,VScalar> DS; // Templatized data set type
typedef Visualization::Wrappers::SlicedScalarVectorDataValue<DS,VScalar> DataValue; // Type of data value descriptor
typedef Visualization::Wrappers::Module<DS,DataValue> BaseModule; // Module base class type

}

class StructuredGocadFile:public BaseModule
	{
	/* Constructors and destructors: */
	public:
	StructuredGocadFile(void); // Default constructor
	
	/* Methods: */
	virtual Visualization::Abstract::DataSet* load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const;
	};

}

}

#endif
