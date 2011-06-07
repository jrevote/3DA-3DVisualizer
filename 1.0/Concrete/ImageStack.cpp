/***********************************************************************
ImageStack - Class to represent scalar-valued Cartesian data sets stored
as stacks of color or greyscale images.
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

#include <ctype.h>
#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Images/RGBImage.h>
#include <Images/ReadImageFile.h>

#include <Concrete/ImageStack.h>

namespace Visualization {

namespace Concrete {

/***************************
Methods of class ImageStack:
***************************/

ImageStack::ImageStack(void)
	:BaseModule("ImageStack")
	{
	}

Visualization::Abstract::DataSet* ImageStack::load(const std::vector<std::string>& args) const
	{
	/* Parse arguments: */
	bool medianFilter=false;
	bool lowpassFilter=false;
	for(unsigned int i=1;i<args.size();++i)
		{
		if(strcasecmp(args[i].c_str(),"MEDIANFILTER")==0)
			medianFilter=true;
		else if(strcasecmp(args[i].c_str(),"LOWPASSFILTER")==0)
			lowpassFilter=true;
		}
	
	/* Open the metadata file: */
	Misc::File file(args[0].c_str(),"rt");
	
	/* Parse the image stack layout: */
	DS::Index numVertices;
	DS::Size cellSize;
	char* sliceDirectory=0;
	char* sliceFileNameTemplate=0;
	int sliceIndexStart=0;
	int sliceIndexFactor=1;
	int regionOrigin[2]={0,0};
	while(!file.eof())
		{
		/* Read the next line from the file: */
		char line[256];
		file.gets(line,sizeof(line));
		
		/* Search for the first equal sign in the line: */
		char* eqPtr=0;
		for(eqPtr=line;*eqPtr!='\0'&&*eqPtr!='=';++eqPtr)
			;
		
		if(*eqPtr=='=')
			{
			/* Extract the tag name to the left of the equal sign: */
			char* tagStart;
			for(tagStart=line;isspace(*tagStart);++tagStart)
				;
			char* tagEnd;
			for(tagEnd=eqPtr;isspace(tagEnd[-1])&&tagEnd>tagStart;--tagEnd)
				;
			
			/* Extract the value to the right of the equal sign: */
			char* valueStart;
			for(valueStart=eqPtr+1;*valueStart!='\0'&&isspace(*valueStart);++valueStart)
				;
			char* valueEnd;
			for(valueEnd=valueStart;*valueEnd!='\0';++valueEnd)
				;
			for(;isspace(valueEnd[-1])&&valueEnd>valueStart;--valueEnd)
				;
			
			if(tagEnd>tagStart&&valueEnd>valueStart)
				{
				*tagEnd='\0';
				*valueEnd='\0';
				if(strcasecmp(tagStart,"numSlices")==0)
					sscanf(valueStart,"%d",&numVertices[0]);
				else if(strcasecmp(tagStart,"imageSize")==0)
					sscanf(valueStart,"%d %d",&numVertices[2],&numVertices[1]);
				else if(strcasecmp(tagStart,"regionOrigin")==0)
					sscanf(valueStart,"%d %d",&regionOrigin[0],&regionOrigin[1]);
				else if(strcasecmp(tagStart,"sampleSpacing")==0)
					sscanf(valueStart,"%f %f %f",&cellSize[0],&cellSize[2],&cellSize[1]);
				else if(strcasecmp(tagStart,"sliceDirectory")==0)
					{
					delete[] sliceDirectory;
					sliceDirectory=new char[valueEnd-valueStart+1];
					strcpy(sliceDirectory,valueStart);
					}
				else if(strcasecmp(tagStart,"sliceFileNameTemplate")==0)
					{
					delete[] sliceFileNameTemplate;
					sliceFileNameTemplate=new char[valueEnd-valueStart+1];
					strcpy(sliceFileNameTemplate,valueStart);
					}
				else if(strcasecmp(tagStart,"sliceIndexStart")==0)
					sscanf(valueStart,"%d",&sliceIndexStart);
				else if(strcasecmp(tagStart,"sliceIndexFactor")==0)
					sscanf(valueStart,"%d",&sliceIndexFactor);
				}
			}
		}
	
	/* Create the data set: */
	DataSet* result=new DataSet;
	result->getDs().setData(numVertices,cellSize);
	
	/* Load all image slices: */
	DataSet::DS::Array& vertices=result->getDs().getVertices();
	unsigned char* vertexPtr=vertices.getArray();
	for(int i=0;i<numVertices[0];++i)
		{
		/* Generate the slice file name: */
		char sliceFileName[1024];
		snprintf(sliceFileName,sizeof(sliceFileName),sliceFileNameTemplate,i*sliceIndexFactor+sliceIndexStart);
		
		/* Load the slice as an RGB image: */
		Images::RGBImage slice=Images::readImageFile(sliceFileName);
		
		/* Check if the slice conforms: */
		if(slice.getSize(0)<(unsigned int)(regionOrigin[0]+numVertices[2])||slice.getSize(1)<(unsigned int)(regionOrigin[1]+numVertices[1]))
			Misc::throwStdErr("ImageStack::load: Size of slice file \"%s\" does not match image stack size",sliceFileName);
		
		/* Convert the slice's pixels to greyscale and copy them into the data set: */
		for(int y=regionOrigin[1];y<regionOrigin[1]+numVertices[1];++y)
			for(int x=regionOrigin[0];x<regionOrigin[0]+numVertices[2];++x,++vertexPtr)
				{
				const Images::RGBImage::Color& pixel=slice.getPixel(x,y);
				float value=float(pixel[0])*0.299f+float(pixel[1])*0.587+float(pixel[2])*0.114f;
				*vertexPtr=(unsigned char)(Math::floor(value+0.5f));
				}
		}
	
	/* Run a median + lowpass filter on all slice triples to reduce random speckle: */
	unsigned char* filtered=new unsigned char[numVertices[0]];
	ptrdiff_t vPtrInc=vertices.getIncrement(0);
	for(int y=0;y<numVertices[1];++y)
		for(int x=0;x<numVertices[2];++x)
			{
			if(medianFilter)
				{
				/* Run median filter: */
				unsigned char* vPtr=vertices.getAddress(0,y,x);
				filtered[0]=*vPtr;
				vPtr+=vPtrInc;
				for(int z=1;z<numVertices[0]-1;++z,vPtr+=vPtrInc)
					{
					if(vPtr[-vPtrInc]<vPtr[0])
						{
						if(vPtr[0]<vPtr[vPtrInc])
							filtered[z]=vPtr[0];
						else
							filtered[z]=vPtr[-vPtrInc]<vPtr[vPtrInc]?vPtr[vPtrInc]:vPtr[-vPtrInc];
						}
					else
						{
						if(vPtr[-vPtrInc]<vPtr[vPtrInc])
							filtered[z]=vPtr[-vPtrInc];
						else
							filtered[z]=vPtr[0]<vPtr[vPtrInc]?vPtr[vPtrInc]:vPtr[0];
						}
					}
				filtered[numVertices[0]-1]=*vPtr;
				}
			else if(lowpassFilter)
				{
				/* Copy the source image data to allow lowpass filtering: */
				unsigned char* vPtr=vertices.getAddress(0,y,x);
				for(int z=0;z<numVertices[0];++z,vPtr+=vPtrInc)
					filtered[z]=*vPtr;
				}
			
			if(lowpassFilter)
				{
				/* Run lowpass filter: */
				unsigned char* vPtr=vertices.getAddress(0,y,x);
				*vPtr=(unsigned char)((int(filtered[0])*3+int(filtered[1])*2+int(filtered[2])+3)/6);
				vPtr+=vPtrInc;
				*vPtr=(unsigned char)((int(filtered[0])*2+int(filtered[1])*3+int(filtered[2])*2+int(filtered[3])+4)/8);
				vPtr+=vPtrInc;
				for(int z=2;z<numVertices[0]-2;++z,vPtr+=vPtrInc)
					*vPtr=(unsigned char)((int(filtered[z-2])+int(filtered[z-1])*2+int(filtered[z])*3+int(filtered[z+1])*2+int(filtered[z+2])+4)/9);
				*vPtr=(unsigned char)((int(filtered[numVertices[0]-4])+int(filtered[numVertices[0]-3])*2+int(filtered[numVertices[0]-2])*3+int(filtered[numVertices[0]-1])*2+4)/8);
				vPtr+=vPtrInc;
				*vPtr=(unsigned char)((int(filtered[numVertices[0]-3])+int(filtered[numVertices[0]-2])*2+int(filtered[numVertices[0]-1])*3+3)/6);
				}
			else if(medianFilter)
				{
				/* Copy the filtered data back to the volume: */
				unsigned char* vPtr=vertices.getAddress(0,y,x);
				for(int z=0;z<numVertices[0];++z,vPtr+=vPtrInc)
					*vPtr=filtered[z];
				}
			}
	
	delete[] filtered;
	
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
	Visualization::Concrete::ImageStack* module=new Visualization::Concrete::ImageStack();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
