/***********************************************************************
CartesianCoordinateTransformer - Coordinate transformer from Cartesian
coordinates.
Part of the abstract interface to the templatized visualization
components.
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

#include <Wrappers/CartesianCoordinateTransformer.h>

namespace Visualization {

namespace Wrappers {

/***********************************************
Methods of class CartesianCoordinateTransformer:
***********************************************/

Visualization::Abstract::CoordinateTransformer* CartesianCoordinateTransformer::clone(void) const
	{
	return new CartesianCoordinateTransformer;
	}

const char* CartesianCoordinateTransformer::getComponentName(int index) const
	{
	const char* result=0;
	switch(index)
		{
		case 0:
			result="X";
			break;
		
		case 1:
			result="Y";
			break;
		
		case 2:
			result="Z";
			break;
		
		default:
			; // Just to make compiler happy
		}
	
	return result;
	}

}

}
