/***********************************************************************
Isosurface - Wrapper class for isosurfaces as visualization elements.
Part of the wrapper layer of the templatized visualization
components.
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

#ifndef VISUALIZATION_WRAPPERS_ISOSURFACE_INCLUDED
#define VISUALIZATION_WRAPPERS_ISOSURFACE_INCLUDED

#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>

#include <Abstract/Element.h>
#if 1
#include <Templatized/IndexedTriangleSet.h>
#else
#include <Templatized/TriangleSet.h>
#endif

/* Forward declarations: */
class GLColorMap;

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class Isosurface:public Visualization::Abstract::Element
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DataSetWrapper::VScalar VScalar; // Scalar type of scalar extractor
	typedef GLVertex<void,0,void,0,Scalar,Scalar,dimension> Vertex; // Data type for triangle vertices
	#if 1
	typedef Visualization::Templatized::IndexedTriangleSet<Vertex> Surface; // Data structure to represent surfaces
	#else
	typedef Visualization::Templatized::TriangleSet<Vertex> Surface; // Data structure to represent surfaces
	#endif
	
	/* Elements: */
	private:
	const GLColorMap* colorMap; // Color map for isosurface vertex values
	VScalar isovalue; // Isovalue for this isosurface
	Surface surface; // Representation of the isosurface
	
	/* Constructors and destructors: */
	public:
	Isosurface(const GLColorMap* sColorMap,VScalar sIsovalue,Comm::MulticastPipe* pipe); // Creates an empty isosurface for the given color map and isovalue
	private:
	Isosurface(const Isosurface& source); // Prohibit copy constructor
	Isosurface& operator=(const Isosurface& source); // Prohibit assignment operator
	public:
	virtual ~Isosurface(void);
	
	/* Methods: */
	const GLColorMap* getColorMap(void) const // Returns the color map
		{
		return colorMap;
		}
	VScalar getIsovalue(void) const // Returns the isovalue
		{
		return isovalue;
		}
	Surface& getSurface(void) // Returns the surface representation
		{
		return surface;
		}
	size_t getElementSize(void) const // Returns the number of triangles in the surface representation
		{
		return surface.getNumTriangles();
		}
	virtual std::string getName(void) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_ISOSURFACE_IMPLEMENTATION
#include <Wrappers/Isosurface.cpp>
#endif

#endif
