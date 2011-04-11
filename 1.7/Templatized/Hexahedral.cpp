/***********************************************************************
Hexahedral - Policy class to select appropriate cell algorithms for a
given data set class.
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

#include <Templatized/Hexahedral.h>

namespace Visualization {

namespace Templatized {

/***********************************
Static elements of class Hexahedral<2>:
***********************************/

const int Hexahedral<2>::edgeVertexIndices[Hexahedral<2>::numEdges][2]=
	{
	{0,1},{0,2},{1,2}
	};

const int Hexahedral<2>::faceVertexIndices[Hexahedral<2>::numFaces][Hexahedral<2>::numFaceVertices]=
	{
	{0,1},{1,2},{2,0}
	};

/***********************************
Static elements of class Hexahedral<3>:
***********************************/

const int Hexahedral<3>::edgeVertexIndices[Hexahedral<3>::numEdges][2]=
	{
	{0,1},{0,2},{0,3},
	{1,2},{1,3},{2,3}
	};

const int Hexahedral<3>::faceVertexIndices[Hexahedral<3>::numFaces][Hexahedral<3>::numFaceVertices]=
	{
	{1,3,2},{0,2,3},
	{0,3,1},{0,1,2}
	};

}

}
