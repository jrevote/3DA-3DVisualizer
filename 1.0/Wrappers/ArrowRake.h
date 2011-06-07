/***********************************************************************
ArrowRake - Class to represent rakes of arrow glyphs as visualization
elements.
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

#ifndef VISUALIZATION_WRAPPERS_ARROWRAKE_INCLUDED
#define VISUALIZATION_WRAPPERS_ARROWRAKE_INCLUDED

#include <Misc/ArrayIndex.h>
#include <Misc/Array.h>
#include <GL/gl.h>
#include <GL/GLObject.h>

#include <Abstract/Element.h>

/* Forward declarations: */
namespace Comm {
class MulticastPipe;
}
class GLColorMap;

namespace Visualization {

namespace Wrappers {

template <class DataSetWrapperParam>
class ArrowRake:public Visualization::Abstract::Element,public GLObject
	{
	/* Embedded classes: */
	public:
	typedef Visualization::Abstract::Element Base; // Base class
	typedef DataSetWrapperParam DataSetWrapper; // Compatible data set type
	typedef typename DataSetWrapper::DS DS; // Type of templatized data set
	typedef typename DS::Scalar Scalar; // Scalar type of data set's domain
	static const int dimension=DS::dimension; // Dimension of data set's domain
	typedef typename DS::Point Point; // Point type in data set's domain
	typedef typename DS::Vector Vector; // Vector type in data set's domain
	
	struct Arrow // Structure representing data for an arrow glyph
		{
		/* Elements: */
		public:
		Point base; // Arrow base point
		bool valid; // Flag if the arrow is valid
		Vector direction; // Vector from arrow base point to arrow tip
		Scalar scalarValue; // Scalar value used to color arrow glyph
		};
	
	typedef Misc::ArrayIndex<2> Index; // Type for rake array indices
	typedef Misc::Array<Arrow,2> Rake; // 2D array of arrows forming a rake
	private:
	typedef GLVertex<void,0,void,0,Scalar,Scalar,dimension> Vertex; // Data type for arrow glyph vertices
	
	struct DataItem:public GLObject::DataItem
		{
		/* Elements: */
		public:
		GLuint vertexBufferId; // ID of buffer object for vertex data
		GLuint indexBufferId; // ID of buffer object for index data
		unsigned int version; // Version number of the arrow glyphs in the buffer objects
		Scalar arrowShaftRadius; // Shaft radius of arrow glyphs in the buffer objects
		
		/* Constructors and destructors: */
		DataItem(void);
		virtual ~DataItem(void);
		};
	
	/* Elements: */
	private:
	Comm::MulticastPipe* pipe; // Pipe to stream arrow rake data in a cluster environment (owned by caller)
	unsigned int version; // Version number of the arrow rake
	const GLColorMap* colorMap; // Color map to color arrow glyphs
	Rake rake; // Array containing the arrow definitions
	GLuint numArrowPoints; // Number of points for arrow glyph creation
	Scalar arrowLengthScale; // Length scaling factor for arrow glyphs
	Scalar arrowShaftRadius; // Shaft radius of arrow glyphs
	
	/* Constructors and destructors: */
	public:
	ArrowRake(const GLColorMap* sColorMap,const Index& sRakeSize,Comm::MulticastPipe* pipe); // Creates an empty arrow rake for the given color map and number of arrows
	private:
	ArrowRake(const ArrowRake& source); // Prohibit copy constructor
	ArrowRake& operator=(const ArrowRake& source); // Prohibit assignment operator
	public:
	virtual ~ArrowRake(void);
	
	/* Methods: */
	const GLColorMap* getColorMap(void) const // Returns the color map
		{
		return colorMap;
		}
	const Index& getRakeSize(void) const // Returns the number of arrow glyphs in the rake
		{
		return rake.getSize();
		}
	const Rake& getRake(void) const // Returns the array of arrows
		{
		return rake;
		}
	Rake& getRake(void) // Ditto
		{
		return rake;
		}
	void update(void) // Updates the rake array
		{
		++version;
		}
	GLuint getNumArrowPoints(void) const // Returns the number of points for arrow glyph rendering
		{
		return numArrowPoints;
		}
	Scalar getArrowLengthScale(void) const // Returns the length scaling factor for arrow glyphs
		{
		return arrowLengthScale;
		}
	Scalar getArrowShaftRadius(void) const // Returns the shaft radius for arrow glyphs
		{
		return arrowShaftRadius;
		}
	void setArrowLengthScale(Scalar newArrowLengthScale); // Sets a new length scaling factor for arrow glyphs
	void setArrowShaftRadius(Scalar newArrowShaftRadius); // Sets a new shaft radius for arrow glyphs
	virtual std::string getName(void) const;
	virtual void initContext(GLContextData& contextData) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	};

}

}

#ifndef VISUALIZATION_WRAPPERS_ARROWRAKE_IMPLEMENTATION
#include <Wrappers/ArrowRake.cpp>
#endif

#endif
