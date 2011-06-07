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

#define VISUALIZATION_WRAPPERS_ARROWRAKE_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Comm/MulticastPipe.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexArrayParts.h>
#include <GL/GLVertex.h>
#include <GL/GLColorMap.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>
#include <Vrui/Vrui.h>

#include <Wrappers/RenderArrow.h>

#include <Wrappers/ArrowRake.h>

namespace Visualization {

namespace Wrappers {

/************************************
Methods of class ArrowRake::DataItem:
************************************/

template <class DataSetWrapperParam>
inline
ArrowRake<DataSetWrapperParam>::DataItem::DataItem(void)
	:vertexBufferId(0),indexBufferId(0),
	 version(0),arrowShaftRadius(0)
	{
	if(GLARBVertexBufferObject::isSupported())
		{
		/* Initialize the vertex buffer object extension: */
		GLARBVertexBufferObject::initExtension();
		
		/* Create a vertex buffer object: */
		glGenBuffersARB(1,&vertexBufferId);
		
		/* Create an index buffer object: */
		glGenBuffersARB(1,&indexBufferId);
		}
	else
		Misc::throwStdErr("ArrowRake::DataItem::DataItem: GL_ARB_vertex_buffer_object extension not supported");
	}

template <class DataSetWrapperParam>
inline
ArrowRake<DataSetWrapperParam>::DataItem::~DataItem(void)
	{
	/* Delete the vertex buffer object: */
	glDeleteBuffersARB(1,&vertexBufferId);
	
	/* Delete the index buffer object: */
	glDeleteBuffersARB(1,&indexBufferId);
	}

/**************************
Methods of class ArrowRake:
**************************/

template <class DataSetWrapperParam>
inline
ArrowRake<DataSetWrapperParam>::ArrowRake(
	const GLColorMap* sColorMap,
	const typename ArrowRake<DataSetWrapperParam>::Index& sRakeSize,
	Comm::MulticastPipe* sPipe)
	:pipe(sPipe),
	 version(0),
	 colorMap(sColorMap),
	 rake(sRakeSize),
	 numArrowPoints(16),
	 arrowLengthScale(1),
	 arrowShaftRadius(0)
	{
	/* Invalidate all arrows: */
	for(Index index(0);index[0]<rake.getSize(0);rake.preInc(index))
		rake(index).valid=false;
	}

template <class DataSetWrapperParam>
inline
ArrowRake<DataSetWrapperParam>::~ArrowRake(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
void
ArrowRake<DataSetWrapperParam>::setArrowLengthScale(
	typename ArrowRake<DataSetWrapperParam>::Scalar newArrowLengthScale)
	{
	arrowLengthScale=newArrowLengthScale;
	++version;
	}

template <class DataSetWrapperParam>
inline
void
ArrowRake<DataSetWrapperParam>::setArrowShaftRadius(
	typename ArrowRake<DataSetWrapperParam>::Scalar newArrowShaftRadius)
	{
	arrowShaftRadius=newArrowShaftRadius;
	++version;
	}

template <class DataSetWrapperParam>
inline
std::string
ArrowRake<DataSetWrapperParam>::getName(
	void) const
	{
	return "Arrow Rake";
	}

template <class DataSetWrapperParam>
inline
void
ArrowRake<DataSetWrapperParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	
	/* Create a vertex buffer and index buffer: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB,rake.getNumElements()*getArrowNumVertices(numArrowPoints)*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,rake.getNumElements()*getArrowNumIndices(numArrowPoints)*sizeof(GLuint),0,GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}

template <class DataSetWrapperParam>
inline
void
ArrowRake<DataSetWrapperParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	/* Set up OpenGL state for isosurface rendering: */
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if(!lightingEnabled)
		glEnable(GL_LIGHTING);
	GLboolean normalizeEnabled=glIsEnabled(GL_NORMALIZE);
	if(!normalizeEnabled)
		glEnable(GL_NORMALIZE);
	GLboolean colorMaterialEnabled=glIsEnabled(GL_COLOR_MATERIAL);
	if(!colorMaterialEnabled)
		glEnable(GL_COLOR_MATERIAL);
	GLMaterial frontMaterial=glGetMaterial(GLMaterialEnums::FRONT);
	glMaterial(GLMaterialEnums::FRONT,GLMaterial(GLMaterial::Color(1.0f,1.0f,1.0f),GLMaterial::Color(0.6f,0.6f,0.6f),25.0f));
	
	/* Bind the buffers: */
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
	
	/* Retrieve the updated arrow shaft radius: */
	Scalar newArrowShaftRadius=Scalar((Vrui::Scalar(0.5)*Vrui::getUiSize())/Vrui::getNavigationTransformation().getScaling());
	
	/* Update the vertex and index buffers: */
	if(dataItem->version!=version||dataItem->arrowShaftRadius!=newArrowShaftRadius)
		{
		/* Map the buffers: */
		Vertex* vertexPtr=static_cast<Vertex*>(glMapBufferARB(GL_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
		GLuint* indexPtr=static_cast<GLuint*>(glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,GL_WRITE_ONLY_ARB));
		
		/* Create glyphs for all arrows in the rake: */
		GLuint vertexBase=0;
		for(Index index(0);index[0]<rake.getSize(0);rake.preInc(index))
			if(rake(index).valid)
				{
				/* Create the arrow glyph: */
				createArrow(rake(index).base,rake(index).direction*arrowLengthScale,newArrowShaftRadius,newArrowShaftRadius*Scalar(3),newArrowShaftRadius*Scalar(6),numArrowPoints,vertexPtr,vertexBase,indexPtr);
				
				/* Move forward in the buffers: */
				vertexBase+=getArrowNumVertices(numArrowPoints);
				indexPtr+=getArrowNumIndices(numArrowPoints);
				}
		
		/* Unmap the buffers: */
		glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
		glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);
		
		dataItem->version=version;
		dataItem->arrowShaftRadius=newArrowShaftRadius;
		}
	
	/* Render all arrow glyphs: */
	glVertexPointer(static_cast<const Vertex*>(0));
	const GLuint* indexPtr=0;
	for(Index index(0);index[0]<rake.getSize(0);rake.preInc(index))
		if(rake(index).valid)
			{
			/* Render the arrow glyph: */
			glColor((*colorMap)(rake(index).scalarValue));
			renderArrow(numArrowPoints,indexPtr);
			indexPtr+=getArrowNumIndices(numArrowPoints);
			}
	
	/* Unbind the buffers: */
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	
	/* Reset OpenGL state: */
	glMaterial(GLMaterialEnums::FRONT,frontMaterial);
	if(!colorMaterialEnabled)
		glDisable(GL_COLOR_MATERIAL);
	if(!normalizeEnabled)
		glDisable(GL_NORMALIZE);
	if(!lightingEnabled)
		glDisable(GL_LIGHTING);
	}

}

}
