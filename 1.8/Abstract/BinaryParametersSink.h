/***********************************************************************
BinaryParametersSink - Generic class for parameter sinks utilizing the
pipe I/O abstraction.
Part of the abstract interface to the templatized visualization
components.
Copyright (c) 2010 Oliver Kreylos

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

#ifndef VISUALIZATION_ABSTRACT_BINARYPARAMETERSSINK_INCLUDED
#define VISUALIZATION_ABSTRACT_BINARYPARAMETERSSINK_INCLUDED

#include <Abstract/VariableManager.h>
#include <Abstract/ParametersSink.h>

namespace Visualization {

namespace Abstract {

template <class DataSinkParam>
class BinaryParametersSink:public ParametersSink
	{
	/* Embedded classes: */
	public:
	typedef DataSinkParam DataSink; // Type for data sinks
	
	/* Elements: */
	private:
	DataSink& sink; // The data sink
	bool raw; // Flag whether the sink writes variable indices (true) or variable names (false)
	
	/* Constructors and destructors: */
	public:
	BinaryParametersSink(const VariableManager* sVariableManager,DataSink& sSink,bool sRaw);
	
	/* Methods from ParametersSink: */
	virtual void write(const char* name,const WriterBase& value);
	virtual void writeScalarVariable(const char* name,int scalarVariableIndex);
	virtual void writeVectorVariable(const char* name,int vectorVariableIndex);
	};

}

}

#endif
