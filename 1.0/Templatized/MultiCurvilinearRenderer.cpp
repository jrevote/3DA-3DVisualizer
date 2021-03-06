/***********************************************************************
MultiCurvilinearRenderer - Class to render multi-grid curvilinear data
sets. Implemented as a specialization of the generic DataSetRenderer
class.
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

#define VISUALIZATION_TEMPLATIZED_MULTICURVILINEARRENDERER_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <GL/GLContextData.h>
#include <GL/GLGeometryWrappers.h>

#include <Templatized/MultiCurvilinearRenderer.h>

namespace Visualization {

namespace Templatized {

namespace MultiCurvilinearRendererImplementation {

/*************************************************************************
Internal helper class to render curvilinear grids of different dimensions:
*************************************************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
class GridRenderer
	{
	/* Dummy class; only dimension-specializations make sense */
	};

template <class ScalarParam,class ValueParam>
class GridRenderer<ScalarParam,2,ValueParam>
	{
	/* Embedded classes: */
	public:
	typedef MultiCurvilinear<ScalarParam,2,ValueParam> DataSet;
	typedef typename DataSet::Box Box;
	typedef typename DataSet::Array Array;
	typedef typename DataSet::Index Index;
	typedef typename DataSet::Cell Cell;
	
	/* Methods: */
	inline static void renderBoundingBox(const Box& box)
		{
		glBegin(GL_LINE_LOOP);
		glVertex(box.getVertex(0));
		glVertex(box.getVertex(1));
		glVertex(box.getVertex(3));
		glVertex(box.getVertex(2));
		glEnd();
		}
	inline static void renderGridOutline(const Array& vertices)
		{
		/* Get vertex array size: */
		const Index& numVertices=vertices.getSize();
		
		Index index;
		
		/* Render grid's outline: */
		glBegin(GL_LINE_STRIP);
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			glVertex(vertices(index).pos);
		glEnd();
		glBegin(GL_LINE_STRIP);
		index[1]=numVertices[1]-1;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			glVertex(vertices(index).pos);
		glEnd();
		glBegin(GL_LINE_STRIP);
		index[0]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			glVertex(vertices(index).pos);
		glEnd();
		glBegin(GL_LINE_STRIP);
		index[0]=numVertices[0]-1;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			glVertex(vertices(index).pos);
		glEnd();
		}
	inline static void renderGridFaces(const Array& vertices,int faceMask)
		{
		/* Get vertex array size: */
		const Index& numVertices=vertices.getSize();
		
		Index index;
		
		/* Render grid's faces: */
		if(faceMask&0x1)
			{
			glBegin(GL_LINE_STRIP);
			index[0]=0;
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(vertices(index).pos);
			glEnd();
			}
		if(faceMask&0x2)
			{
			glBegin(GL_LINE_STRIP);
			index[0]=numVertices[0]-1;
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(vertices(index).pos);
			glEnd();
			}
		if(faceMask&0x4)
			{
			glBegin(GL_LINE_STRIP);
			index[1]=0;
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(vertices(index).pos);
			glEnd();
			}
		if(faceMask&0x8)
			{
			glBegin(GL_LINE_STRIP);
			index[1]=numVertices[1]-1;
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(vertices(index).pos);
			glEnd();
			}
		}
	inline static void renderGridCells(const Array& vertices)
		{
		/* Get vertex array size: */
		const Index& numVertices=vertices.getSize();
		
		Index index;
		
		/* Render grid lines along y direction: */
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			glBegin(GL_LINE_STRIP);
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(vertices(index).pos);
			glEnd();
			}
		
		/* Render grid lines along x direction: */
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			glBegin(GL_LINE_STRIP);
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(vertices(index).pos);
			glEnd();
			}
		}
	inline static void highlightCell(const Cell& cell)
		{
		glBegin(GL_LINE_LOOP);
		glVertex(cell.getVertexPosition(0));
		glVertex(cell.getVertexPosition(1));
		glVertex(cell.getVertexPosition(3));
		glVertex(cell.getVertexPosition(2));
		glEnd();
		}
	};

template <class ScalarParam,class ValueParam>
class GridRenderer<ScalarParam,3,ValueParam>
	{
	/* Embedded classes: */
	public:
	typedef MultiCurvilinear<ScalarParam,3,ValueParam> DataSet;
	typedef typename DataSet::Box Box;
	typedef typename DataSet::Array Array;
	typedef typename DataSet::Index Index;
	typedef typename DataSet::Cell Cell;
	
	/* Methods: */
	inline static void renderBoundingBox(const Box& box)
		{
		glBegin(GL_LINE_STRIP);
		glVertex(box.getVertex(0));
		glVertex(box.getVertex(1));
		glVertex(box.getVertex(3));
		glVertex(box.getVertex(2));
		glVertex(box.getVertex(0));
		glVertex(box.getVertex(4));
		glVertex(box.getVertex(5));
		glVertex(box.getVertex(7));
		glVertex(box.getVertex(6));
		glVertex(box.getVertex(4));
		glEnd();
		glBegin(GL_LINES);
		glVertex(box.getVertex(1));
		glVertex(box.getVertex(5));
		glVertex(box.getVertex(3));
		glVertex(box.getVertex(7));
		glVertex(box.getVertex(2));
		glVertex(box.getVertex(6));
		glEnd();
		}
	inline static void renderGridLine(const Array& vertices,const Index& startIndex,int axis)
		{
		/* Get pointer to first grid vertex: */
		const typename DataSet::GridVertex* vPtr=vertices.getAddress(startIndex);
		int numVertices=vertices.getSize(axis);
		int stride=vertices.getIncrement(axis);
		
		/* Render grid line: */
		glBegin(GL_LINE_STRIP);
		for(int i=0;i<numVertices;++i,vPtr+=stride)
			glVertex(vPtr->pos);
		glEnd();
		}
	inline static void renderGridOutline(const Array& vertices)
		{
		/* Get vertex array size: */
		const Index& numVertices=vertices.getSize();
		
		/* Render grid outlines: */
		renderGridLine(vertices,Index(0,0,0),0);
		renderGridLine(vertices,Index(0,numVertices[1]-1,0),0);
		renderGridLine(vertices,Index(0,numVertices[1]-1,numVertices[2]-1),0);
		renderGridLine(vertices,Index(0,0,numVertices[2]-1),0);
		renderGridLine(vertices,Index(0,0,0),1);
		renderGridLine(vertices,Index(numVertices[0]-1,0,0),1);
		renderGridLine(vertices,Index(numVertices[0]-1,0,numVertices[2]-1),1);
		renderGridLine(vertices,Index(0,0,numVertices[2]-1),1);
		renderGridLine(vertices,Index(0,0,0),2);
		renderGridLine(vertices,Index(numVertices[0]-1,0,0),2);
		renderGridLine(vertices,Index(numVertices[0]-1,numVertices[1]-1,0),2);
		renderGridLine(vertices,Index(0,numVertices[1]-1,0),2);
		}
	inline static void renderGridFaces(const Array& vertices,int faceMask)
		{
		/* Get vertex array size: */
		const Index& numVertices=vertices.getSize();
		
		Index index;
		
		/* Render grid lines in (y,z)-plane: */
		index[1]=0;
		for(index[2]=0;index[2]<numVertices[2];++index[2])
			{
			index[0]=0;
			if(faceMask&0x01)
				renderGridLine(vertices,index,1);
			index[0]=numVertices[0]-1;
			if(faceMask&0x02)
				renderGridLine(vertices,index,1);
			}
		index[2]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			index[0]=0;
			if(faceMask&0x01)
				renderGridLine(vertices,index,2);
			index[0]=numVertices[0]-1;
			if(faceMask&0x02)
				renderGridLine(vertices,index,2);
			}
		
		/* Render grid lines in (x,z)-plane: */
		index[0]=0;
		for(index[2]=0;index[2]<numVertices[2];++index[2])
			{
			index[1]=0;
			if(faceMask&0x04)
				renderGridLine(vertices,index,0);
			index[1]=numVertices[1]-1;
			if(faceMask&0x08)
				renderGridLine(vertices,index,0);
			}
		index[2]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			index[1]=0;
			if(faceMask&0x04)
				renderGridLine(vertices,index,2);
			index[1]=numVertices[1]-1;
			if(faceMask&0x08)
				renderGridLine(vertices,index,2);
			}
		
		/* Render grid lines in (x,y)-plane: */
		index[0]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			index[2]=0;
			if(faceMask&0x10)
				renderGridLine(vertices,index,0);
			index[2]=numVertices[2]-1;
			if(faceMask&0x20)
				renderGridLine(vertices,index,0);
			}
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			index[2]=0;
			if(faceMask&0x10)
				renderGridLine(vertices,index,1);
			index[2]=numVertices[2]-1;
			if(faceMask&0x20)
				renderGridLine(vertices,index,1);
			}
		}
	inline static void renderGridCells(const Array& vertices)
		{
		/* Get vertex array size: */
		const Index& numVertices=vertices.getSize();
		
		Index index;
		
		/* Render grid lines along z-axis: */
		index[2]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				renderGridLine(vertices,index,2);
		
		/* Render grid lines along y-axis: */
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				renderGridLine(vertices,index,1);
		
		/* Render grid lines along x-axis: */
		index[0]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				renderGridLine(vertices,index,0);
		}
	inline static void highlightCell(const Cell& cell)
		{
		glBegin(GL_LINE_STRIP);
		glVertex(cell.getVertexPosition(0));
		glVertex(cell.getVertexPosition(1));
		glVertex(cell.getVertexPosition(3));
		glVertex(cell.getVertexPosition(2));
		glVertex(cell.getVertexPosition(0));
		glVertex(cell.getVertexPosition(4));
		glVertex(cell.getVertexPosition(5));
		glVertex(cell.getVertexPosition(7));
		glVertex(cell.getVertexPosition(6));
		glVertex(cell.getVertexPosition(4));
		glEnd();
		glBegin(GL_LINES);
		glVertex(cell.getVertexPosition(1));
		glVertex(cell.getVertexPosition(5));
		glVertex(cell.getVertexPosition(3));
		glVertex(cell.getVertexPosition(7));
		glVertex(cell.getVertexPosition(2));
		glVertex(cell.getVertexPosition(6));
		glEnd();
		}
	};

}

/**************************************************
Methods of class DataSetRenderer<MultiCurvilinear>:
**************************************************/

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::DataSetRenderer(
	const typename DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::DataSet* sDataSet)
	:dataSet(sDataSet),
	 renderingModeIndex(0)
	{
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::~DataSetRenderer(
	void)
	{
	/* Nothing to do yet... */
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
int
DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::getNumRenderingModes(
	void)
	{
	return 5;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
const char*
DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::getRenderingModeName(
	int renderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=5)
		Misc::throwStdErr("DataSetRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	
	static const char* renderingModeNames[5]=
		{
		"Bounding Box","Grid Outline","Grid Boundary Faces","Grid Faces","Grid Cells"
		};
	
	return renderingModeNames[renderingModeIndex];
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::setRenderingMode(
	int newRenderingModeIndex)
	{
	if(newRenderingModeIndex<0||newRenderingModeIndex>=5)
		Misc::throwStdErr("DataSetRenderer::setRenderingMode: invalid rendering mode index %d",newRenderingModeIndex);
	
	renderingModeIndex=newRenderingModeIndex;
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::glRenderAction(
	GLContextData& contextData) const
	{
	switch(renderingModeIndex)
		{
		case 0:
			/* Render the data set's bounding box: */
			MultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderBoundingBox(dataSet->getDomainBox());
			break;
		
		case 1:
			/* Render each grid's outline: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				MultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridOutline(dataSet->getGrid(gridIndex).getVertices());
			break;
		
		case 2:
			{
			/* Render each grid's faces: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				{
				int faceMask=0x0;
				for(int faceIndex=0;faceIndex<dimensionParam*2;++faceIndex)
					if(dataSet->isBoundaryFace(gridIndex,faceIndex))
						faceMask|=1<<faceIndex;
				MultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridFaces(dataSet->getGrid(gridIndex).getVertices(),faceMask);
				}
			break;
			}
		
		case 3:
			/* Render each grid's faces: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				MultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridFaces(dataSet->getGrid(gridIndex).getVertices(),~0x0);
			break;
		
		case 4:
			/* Render each grid's cells: */
			for(int gridIndex=0;gridIndex<dataSet->getNumGrids();++gridIndex)
				MultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::renderGridCells(dataSet->getGrid(gridIndex).getVertices());
			break;
		}
	}

template <class ScalarParam,int dimensionParam,class ValueParam>
inline
void
DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::renderCell(
	const typename DataSetRenderer<MultiCurvilinear<ScalarParam,dimensionParam,ValueParam> >::CellID& cellID,
	GLContextData& contextData) const
	{
	/* Highlight the cell: */
	MultiCurvilinearRendererImplementation::GridRenderer<ScalarParam,dimensionParam,ValueParam>::highlightCell(dataSet->getCell(cellID));
	}

}

}
