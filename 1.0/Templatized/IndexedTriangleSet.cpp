/***********************************************************************
IndexedTriangleSet - Class to represent surfaces as sets of triangles
sharing vertices.
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

#define VISUALIZATION_TEMPLATIZED_INDEXEDTRIANGLESET_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Comm/MulticastPipe.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/GLVertexArrayParts.h>
#define NONSTANDARD_GLVERTEX_TEMPLATES
#include <GL/GLVertex.h>
#include <GL/GLContextData.h>
#include <GL/GLExtensionManager.h>
#include <GL/Extensions/GLARBVertexBufferObject.h>

#include <Templatized/IndexedTriangleSet.h>

namespace Visualization {

namespace Templatized {

/*********************************************
Methods of class IndexedTriangleSet::DataItem:
*********************************************/

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::DataItem::DataItem(
	void)
	:vertexBufferId(0),indexBufferId(0),
	 version(0),
	 numVertices(0),numTriangles(0)
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
		Misc::throwStdErr("IndexedTriangleSet::DataItem::DataItem: GL_ARB_vertex_buffer_object extension not supported");
	}

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::DataItem::~DataItem(
	void)
	{
	if(vertexBufferId!=0||indexBufferId!=0)
		{
		/* Delete the vertex buffer object: */
		glDeleteBuffersARB(1,&vertexBufferId);
		
		/* Delete the index buffer object: */
		glDeleteBuffersARB(1,&indexBufferId);
		}
	}

/***********************************
Methods of class IndexedTriangleSet:
***********************************/

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::addNewVertexChunk(
	void)
	{
	if(pipe!=0&&vertexTail!=0)
		{
		/* Send the last chunk of vertices across the pipe: */
		size_t numSendVertices=vertexChunkSize-sentTailVertices;
		if(numSendVertices>0)
			{
			pipe->write<unsigned int>((unsigned int)numSendVertices);
			pipe->write<unsigned int>(0U);
			pipe->write<Vertex>(vertexTail->vertices+sentTailVertices,numSendVertices);
			pipe->finishMessage();
			}
		}
	
	/* Add a new vertex chunk to the buffer: */
	VertexChunk* newVertexChunk=new VertexChunk;
	if(vertexTail!=0)
		vertexTail->succ=newVertexChunk;
	else
		vertexHead=newVertexChunk;
	vertexTail=newVertexChunk;
	
	/* Set up the vertex pointer: */
	numVerticesLeft=vertexChunkSize;
	sentTailVertices=0;
	nextVertex=vertexTail->vertices;
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::addNewIndexChunk(
	void)
	{
	if(pipe!=0&&indexTail!=0)
		{
		/* Determine the number of vertices and triangles to send across the pipe: */
		size_t numSendVertices=(vertexChunkSize-numVerticesLeft)-sentTailVertices;
		pipe->write<unsigned int>((unsigned int)numSendVertices);
		pipe->write<unsigned int>((unsigned int)indexChunkSize);
		
		/* Send all vertices in the current tail chunk: */
		if(numSendVertices>0)
			{
			pipe->write<Vertex>(vertexTail->vertices+sentTailVertices,numSendVertices);
			sentTailVertices+=numSendVertices;
			}
		
		/* Send the last chunk of triangles across the pipe: */
		pipe->write<Index>(indexTail->indices,indexChunkSize*3);
		pipe->finishMessage();
		}
	
	/* Add a new index chunk to the buffer: */
	IndexChunk* newIndexChunk=new IndexChunk;
	if(indexTail!=0)
		indexTail->succ=newIndexChunk;
	else
		indexHead=newIndexChunk;
	indexTail=newIndexChunk;
	
	/* Set up the triangle (vertex triple) pointer: */
	numTrianglesLeft=indexChunkSize;
	nextTriangle=indexTail->indices;
	}

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::IndexedTriangleSet(
	Comm::MulticastPipe* sPipe)
	:pipe(sPipe),
	 version(0),
	 numVertices(0),numTriangles(0),
	 vertexHead(0),vertexTail(0),
	 indexHead(0),indexTail(0),
	 numVerticesLeft(0),sentTailVertices(0),numTrianglesLeft(0),
	 nextVertex(0),nextTriangle(0)
	{
	}

template <class VertexParam>
inline
IndexedTriangleSet<VertexParam>::~IndexedTriangleSet(
	void)
	{
	/* Delete all vertex chunks: */
	while(vertexHead!=0)
		{
		VertexChunk* succ=vertexHead->succ;
		delete vertexHead;
		vertexHead=succ;
		}
	
	/* Delete all index chunks: */
	while(indexHead!=0)
		{
		IndexChunk* succ=indexHead->succ;
		delete indexHead;
		indexHead=succ;
		}
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::initContext(
	GLContextData& contextData) const
	{
	/* Create a new context data item: */
	DataItem* dataItem=new DataItem;
	contextData.addDataItem(this,dataItem);
	}

template <class VertexParam>
inline
bool
IndexedTriangleSet<VertexParam>::receive(
	void)
	{
	/* Read the number of vertices and triangles in the next batch: */
	size_t numBatchVertices=pipe->read<unsigned int>();
	size_t numBatchTriangles=pipe->read<unsigned int>();
	
	/* Stop reading if triangle set is fully received: */
	if(numBatchVertices==0&&numBatchTriangles==0)
		return true;
	
	/* Read the vertex data one chunk at a time: */
	while(numBatchVertices>0)
		{
		if(numVerticesLeft==0)
			{
			/* Add a new vertex chunk to the buffer: */
			VertexChunk* newVertexChunk=new VertexChunk;
			if(vertexTail!=0)
				vertexTail->succ=newVertexChunk;
			else
				vertexHead=newVertexChunk;
			vertexTail=newVertexChunk;
			
			/* Set up the vertex pointer: */
			numVerticesLeft=vertexChunkSize;
			nextVertex=vertexTail->vertices;
			}
		
		/* Receive as many vertices as the current chunk can hold: */
		size_t numReadVertices=numBatchVertices;
		if(numReadVertices>numVerticesLeft)
			numReadVertices=numVerticesLeft;
		pipe->read<Vertex>(nextVertex,numReadVertices);
		numBatchVertices-=numReadVertices;
		
		/* Update the vertex storage: */
		numVertices+=numReadVertices;
		numVerticesLeft-=numReadVertices;
		nextVertex+=numReadVertices;
		}
	
	/* Read the triangle data one chunk at a time: */
	while(numBatchTriangles>0)
		{
		if(numTrianglesLeft==0)
			{
			/* Add a new index chunk to the buffer: */
			IndexChunk* newIndexChunk=new IndexChunk;
			if(indexTail!=0)
				indexTail->succ=newIndexChunk;
			else
				indexHead=newIndexChunk;
			indexTail=newIndexChunk;
			
			/* Set up the triangle (vertex triple) pointer: */
			numTrianglesLeft=indexChunkSize;
			nextTriangle=indexTail->indices;
			}
		
		/* Receive as many triangles as the current chunk can hold: */
		size_t numReadTriangles=numBatchTriangles;
		if(numReadTriangles>numTrianglesLeft)
			numReadTriangles=numTrianglesLeft;
		pipe->read<Index>(nextTriangle,numReadTriangles*3);
		numBatchTriangles-=numReadTriangles;
		
		/* Update the triangle storage: */
		numTriangles+=numReadTriangles;
		numTrianglesLeft-=numReadTriangles;
		nextTriangle+=numReadTriangles*3;
		}
	
	return false;
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::clear(
	void)
	{
	++version;
	numVertices=0;
	numTriangles=0;
	
	/* Delete all vertex chunks: */
	while(vertexHead!=0)
		{
		VertexChunk* succ=vertexHead->succ;
		delete vertexHead;
		vertexHead=succ;
		}
	vertexTail=0;
	numVerticesLeft=0;
	nextVertex=0;
	
	/* Delete all index chunks: */
	while(indexHead!=0)
		{
		IndexChunk* succ=indexHead->succ;
		delete indexHead;
		indexHead=succ;
		}
	indexTail=0;
	numTrianglesLeft=0;
	nextTriangle=0;
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::finish(
	void)
	{
	if(pipe!=0)
		{
		/* Send the last batch of vertices and triangles across the pipe: */
		if((vertexTail!=0&&numVerticesLeft<vertexChunkSize)||(indexTail!=0&&numTrianglesLeft<indexChunkSize))
			{
			/* Send the number of vertices and triangles in this batch: */
			size_t numTailVertices=vertexTail!=0?(vertexChunkSize-numVerticesLeft)-sentTailVertices:0;
			size_t numTailTriangles=indexTail!=0?indexChunkSize-numTrianglesLeft:0;
			pipe->write<unsigned int>((unsigned int)numTailVertices);
			pipe->write<unsigned int>((unsigned int)numTailTriangles);
			
			/* Send the vertex data: */
			if(numTailVertices>0)
				pipe->write<Vertex>(vertexTail->vertices+sentTailVertices,numTailVertices);
			
			/* Send the triangle data: */
			if(numTailTriangles>0)
				pipe->write<Index>(indexTail->indices,numTailTriangles*3);
			}
		
		/* Signal that the triangle set is finished: */
		pipe->write<unsigned int>(0);
		pipe->write<unsigned int>(0);
		pipe->finishMessage();
		}
	}

template <class VertexParam>
inline
void
IndexedTriangleSet<VertexParam>::glRenderAction(
	GLContextData& contextData) const
	{
	/* Get the context data item: */
	DataItem* dataItem=contextData.template retrieveDataItem<DataItem>(this);
	
	/* Save the current number of vertices and triangles (for parallel creation and rendering): */
	size_t numRenderTriangles=numTriangles;
	size_t numRenderVertices=numVertices;
	
	/* Render the current amount of triangles: */
	GLVertexArrayParts::enable(Vertex::getPartsMask());
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,dataItem->vertexBufferId);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,dataItem->indexBufferId);
	
	/* Update the vertex and index buffers: */
	if(dataItem->version!=version||dataItem->numVertices!=numRenderVertices)
		{
		/* Upload the vertex data into the vertex buffer: */
		glBufferDataARB(GL_ARRAY_BUFFER_ARB,numRenderVertices*sizeof(Vertex),0,GL_STATIC_DRAW_ARB);
		GLintptrARB offset=0;
		size_t verticesToCopy=numRenderVertices;
		for(const VertexChunk* chPtr=vertexHead;verticesToCopy>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of vertices in this chunk: */
			size_t numChunkVertices=verticesToCopy;
			if(numChunkVertices>vertexChunkSize)
				numChunkVertices=vertexChunkSize;
			
			/* Upload the vertices: */
			glBufferSubDataARB(GL_ARRAY_BUFFER_ARB,offset,numChunkVertices*sizeof(Vertex),chPtr->vertices);
			verticesToCopy-=numChunkVertices;
			offset+=numChunkVertices*sizeof(Vertex);
			}
		dataItem->numVertices=numRenderVertices;
		}
	
	if(dataItem->version!=version||dataItem->numTriangles!=numRenderTriangles)
		{
		/* Upload the index data into the index buffer: */
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,numRenderTriangles*3*sizeof(Index),0,GL_STATIC_DRAW_ARB);
		GLintptrARB offset=0;
		size_t trianglesToCopy=numRenderTriangles;
		for(const IndexChunk* chPtr=indexHead;trianglesToCopy>0;chPtr=chPtr->succ)
			{
			/* Calculate the number of triangles in this chunk: */
			size_t numChunkTriangles=trianglesToCopy;
			if(numChunkTriangles>indexChunkSize)
				numChunkTriangles=indexChunkSize;
			
			/* Upload the vertex indices: */
			glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB,offset,numChunkTriangles*3*sizeof(Index),chPtr->indices);
			trianglesToCopy-=numChunkTriangles;
			offset+=numChunkTriangles*3*sizeof(Index);
			}
		dataItem->numTriangles=numRenderTriangles;
		}
	
	dataItem->version=version;
	
	/* Render the triangles: */
	glVertexPointer(static_cast<const Vertex*>(0));
	// glDrawRangeElements(GL_TRIANGLES,0,GLuint(numRenderVertices)-1,numRenderTriangles*3,GL_UNSIGNED_INT,static_cast<const Index*>(0));
	glDrawElements(GL_TRIANGLES,numRenderTriangles*3,GL_UNSIGNED_INT,static_cast<const Index*>(0));
	
	glBindBufferARB(GL_ARRAY_BUFFER_ARB,0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	GLVertexArrayParts::disable(Vertex::getPartsMask());
	}

}

}
