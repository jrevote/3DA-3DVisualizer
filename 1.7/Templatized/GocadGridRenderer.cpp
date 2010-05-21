/***********************************************************************
GocadGridRenderer - Helper class to render curvilinear grids.
Copyright (c) 2009 Oliver Kreylos

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

#define VISUALIZATION_TEMPLATIZED_GOCADGRIDRENDERER_IMPLEMENTATION

#include <Templatized/GocadGridRenderer.h>

#include <Misc/ThrowStdErr.h>
#include <GL/gl.h>
#include <GL/GLGeometryWrappers.h>

namespace Visualization {

namespace Templatized {

namespace GocadGridRendererImplementation {

/*************************************************************************
Internal helper class to render curvilinear grids of different dimensions:
*************************************************************************/

template <int dimensionParam,class DataSetParam>
class GridRenderer
	{
	/* Dummy class; only dimension-specializations make sense */
	};

template <class DataSetParam>
class GridRenderer<2,DataSetParam>
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet;
	typedef typename DataSet::Scalar Scalar;
	typedef typename DataSet::Point Point;
	typedef typename DataSet::Box Box;
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
	inline static void renderGridOutline(const DataSet* dataSet)
		{
		const Index& numVertices=dataSet->getNumVertices();
		Index index;
		
		/* Render grid's outline: */
		glBegin(GL_LINE_LOOP);
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0]-1;++index[0])
			glVertex(dataSet->getVertexPosition(index));
		for(index[1]=0;index[1]<numVertices[1]-1;++index[1])
			glVertex(dataSet->getVertexPosition(index));
		for(index[0]=numVertices[0]-1;index[0]>0;--index[0])
			glVertex(dataSet->getVertexPosition(index));
		for(index[1]=numVertices[1]-1;index[1]>0;--index[1])
			glVertex(dataSet->getVertexPosition(index));
		glEnd();
		}
	inline static void renderGridFaces(const DataSet* dataSet)
		{
		/* Render the grid outline: */
		renderGridOutline(dataSet);
		}
	inline static void renderGridCells(const DataSet* dataSet)
		{
		const Index& numVertices=dataSet->getNumVertices();
		Index index;
		
		/* Render grid lines along x direction: */
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			{
			glBegin(GL_LINE_STRIP);
			for(index[0]=0;index[0]<numVertices[0];++index[0])
				glVertex(dataSet->getVertexPosition(index));
			glEnd();
			}
		
		/* Render grid lines along y direction: */
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			{
			glBegin(GL_LINE_STRIP);
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				glVertex(dataSet->getVertexPosition(index));
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

template <class DataSetParam>
class GridRenderer<3,DataSetParam>
	{
	/* Embedded classes: */
	public:
	typedef DataSetParam DataSet;
	typedef typename DataSet::Scalar Scalar;
	typedef typename DataSet::Point Point;
	typedef typename DataSet::Box Box;
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
	inline static void renderVox(const DataSet* dataSet,const Index& startIndex,int axis)
		{
		/* Render point: */
      //glColor3f(0.75f, 0.25f, 0.75f); 
      glPointSize(2.5f);
		Index index=startIndex;
		for(index[axis]=0;index[axis]<dataSet->getNumVertices()[axis];++index[axis])
          {   /*glVertex(dataSet->getVertexPosition(index)[0],dataSet->getVertexPosition(index)[1],dataSet->getVertexPosition(index)[2]);*/
	           glBegin(GL_LINE_STRIP);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]+200);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]+200);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]+200);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]+200);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]+200);
              glEnd();
              glBegin(GL_LINES);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]-200,dataSet->getVertexPosition(index)[2]+200);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]+200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]+200);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]-200);
              glVertex(dataSet->getVertexPosition(index)[0]-200,dataSet->getVertexPosition(index)[1]+200,dataSet->getVertexPosition(index)[2]+200);
		        glEnd();
          }
		}
	inline static void renderVoxset(const DataSet* dataSet)
		{
		const Index& numVertices=dataSet->getNumVertices();
		Index index;
		
		/* Render points along x-axis: */
		index[0]=0;
		for(index[1]=0;index[1]<numVertices[1];++index[1])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				renderVox(dataSet,index,0);
		
		/* Render points along y-axis: */
		index[1]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[2]=0;index[2]<numVertices[2];++index[2])
				renderVox(dataSet,index,1);
		
		/* Render points along z-axis: */
		index[2]=0;
		for(index[0]=0;index[0]<numVertices[0];++index[0])
			for(index[1]=0;index[1]<numVertices[1];++index[1])
				renderVox(dataSet,index,2);
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

/****************************************
Methods of class GocadGridRenderer:
****************************************/

template <class DataSetParam>
inline
GocadGridRenderer<DataSetParam>::GocadGridRenderer(
	const typename GocadGridRenderer<DataSetParam>::DataSet* sDataSet)
	:dataSet(sDataSet),
	 renderingModeIndex(0)
	{
	}

template <class DataSetParam>
inline
int
GocadGridRenderer<DataSetParam>::getNumRenderingModes(
	void)
	{
	return 2;
	}

template <class DataSetParam>
inline
const char*
GocadGridRenderer<DataSetParam>::getRenderingModeName(
	int renderingModeIndex)
	{
	if(renderingModeIndex<0||renderingModeIndex>=3)
		Misc::throwStdErr("GocadGridRenderer::getRenderingModeName: invalid rendering mode index %d",renderingModeIndex);
	
	static const char* renderingModeNames[5]=
		{
		"Bounding Box", "Voxets"
		};
	
	return renderingModeNames[renderingModeIndex];
	}

template <class DataSetParam>
inline
void
GocadGridRenderer<DataSetParam>::setRenderingMode(
	int newRenderingModeIndex)
	{
	if(newRenderingModeIndex<0||newRenderingModeIndex>=3)
		Misc::throwStdErr("GocadGridRenderer::setRenderingMode: invalid rendering mode index %d",newRenderingModeIndex);
	
	renderingModeIndex=newRenderingModeIndex;
	}

template <class DataSetParam>
inline
void
GocadGridRenderer<DataSetParam>::glRenderAction(
	GLContextData& contextData) const
	{
	switch(renderingModeIndex)
		{
		case 0:
			/* Render the grid's bounding box: */
			GocadGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderBoundingBox(dataSet->getDomainBox());
			break;

		case 1:
			/* Render the grid's cells: */
			GocadGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::renderVoxset(dataSet);
			break;
		}
	}

template <class DataSetParam>
inline
void
GocadGridRenderer<DataSetParam>::renderCell(
	const typename GocadGridRenderer<DataSetParam>::CellID& cellID,
	GLContextData& contextData) const
	{
	/* Highlight the cell: */
	GocadGridRendererImplementation::GridRenderer<DataSetParam::dimension,DataSetParam>::highlightCell(dataSet->getCell(cellID));
	}

}

}
