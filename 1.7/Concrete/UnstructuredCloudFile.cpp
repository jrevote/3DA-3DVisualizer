/***********************************************************************
UnstructuredCloudFile - Class to read unstructured mesh data in NASA
Cloud format.
Copyright (c) 2004-2007 Oliver Kreylos

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
:q
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/UnstructuredCloudFile.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper functions:
****************/

void readGrid(UnstructuredCloudFile::DS* dataSet,const char* gridFileName)
	{
	/* Open the grid file: */
	Misc::File gridFile(gridFileName,"rt");
   char line[256];
   unsigned int lineIndex = 0;
   int parsedHeaderLines = 0;
	int numVertices;
   int numTriangles;
   int numTetrahedra;
   int numHexahedron;
	
	/* Read the grid file header: */
   while(parsedHeaderLines < 3)
      {
          gridFile.gets(line, sizeof(line));
          ++lineIndex;
          if(sscanf(line, "%d", &numVertices) != 1)
              Misc::throwStdErr("UnstructuredCloudFile::load: invalid vertices count in line %u in grid file %s", lineIndex, gridFileName);
          //if(sscanf(line, "%d", &numHexahedron) != 1)
          //    Misc::throwStdErr("UnstructuredCloudFile::load: invalid hexahedron size in line %u in grid file %s", lineIndex, gridFileName);
          ++parsedHeaderLines;
          if(sscanf(line, "%d", &numTriangles) != 1)
              Misc::throwStdErr("UnstructuredCloudFile::load: invalid triangles count in line %u in grid file %s", lineIndex, gridFileName);
          ++parsedHeaderLines;
          if(sscanf(line, "%d", &numTetrahedra) != 1)
              Misc::throwStdErr("UnstructuredCloudFile::load: invalid tetrahedra count in line %u in grid file %s", lineIndex, gridFileName);
          ++parsedHeaderLines;
      }
   
	/* Add all (uninitialized) vertices to the data set: */
	UnstructuredCloudFile::DS::GridVertexIterator* vertices = new UnstructuredCloudFile::DS::GridVertexIterator[numVertices];
	for(int i = 0; i < numVertices; ++i)
		vertices[i] = dataSet->addVertex(UnstructuredCloudFile::DS::Point(),UnstructuredCloudFile::DS::Value());
	
	/* Read the vertices' coordinates: */
	float* vertexCoords = new float[numVertices];
	for(int coord = 0; coord < 3; ++coord)
		{
		gridFile.read(vertexCoords,numVertices);
		for(int i = 0; i < numVertices; ++i)
			vertices[i]->pos[coord] = vertexCoords[i];
		}
	delete[] vertexCoords;
	
	/* Skip the triangle data: */
	gridFile.seekCurrent(numTriangles*4*sizeof(int));
	
	/* Read tetrahedra indices: */
	int* tetVertexIndices=new int[numTetrahedra*4];
	gridFile.read(tetVertexIndices,numTetrahedra*4);

   //for(int i = 0; i < numHexahedron; i++)	
   //   {
   //   UnstructuredCloudFile::DS::GridVertexIterator cellVertices[6];
   //   for(int j = 0; j < 6; ++j)
   //      cellVertices[j] = vertices[tetVertexIndices[i * 6 + j] - 1]; 
   //   }
	/* Add all tetrahedra to the data set: */
	for(int i=0;i<numTetrahedra;++i)
		{
		/* Convert the one-based indices to vertex iterators: */
		UnstructuredCloudFile::DS::GridVertexIterator cellVertices[4];
		for(int j=0;j<4;++j)
			cellVertices[j]=vertices[tetVertexIndices[i*4+j]-1];
		
		/* Add the cell: */
		dataSet->addCell(cellVertices);
		}
	
	/* Delete temporary data: */
	//delete[] tetVertexIndices;
	delete[] vertices;
	
	/* Finalize the mesh structure: */
	dataSet->finalizeGrid();
	}

}

/***************************************
Methods of class UnstructuredCloudFile:
***************************************/

UnstructuredCloudFile::UnstructuredCloudFile(void)
	:BaseModule("UnstructuredCloudFile")
	{
	}

Visualization::Abstract::DataSet* UnstructuredCloudFile::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Create result data set: */
	DataSet* result=new DataSet;
	
	/* Read the grid structure: */
	char gridFilename[1024];
	//snprintf(gridFilename,sizeof(gridFilename),"%s.grid",args[0].c_str());
	//readGrid(&result->getDs(),gridFilename);
	readGrid(&result->getDs(),args[0].c_str());
	
	return result;
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::UnstructuredCloudFile* module=new Visualization::Concrete::UnstructuredCloudFile();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
