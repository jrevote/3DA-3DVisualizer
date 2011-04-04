/***********************************************************************
UnderworldHDF5File - Class to encapsulate operations on files generated by the
CITCOMT simulation code.
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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Misc/LargeFile.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>
#include <Geometry/Endianness.h>
#include <Concrete/EarthDataSet.h>
#include <Concrete/UnderworldHDF5File.h>
#include <H5Cpp.h>

namespace Visualization {

namespace Concrete {

/****************************
Methods of class UnderworldHDF5File:
****************************/

UnderworldHDF5File::UnderworldHDF5File(void)
	:BaseModule("UnderworldHDF5File")
	{
	}

Visualization::Abstract::DataSet* UnderworldHDF5File::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
	/* Create result data set: */
   DataSet* result = new DataSet;

   /* Open/read the mesh dataset: */
   int meshSize[3];
   std::cout<<"Reading Mesh file...\n"<<std::flush;
   hid_t meshFile=H5Fopen(args[0].c_str(),H5F_ACC_RDONLY,H5P_DEFAULT);
   hid_t meshGroupID=H5Gopen(meshFile,"/");
   H5O_info_t meshInfo;
   H5Oget_info(meshGroupID,&meshInfo);
   std::cout<<"---Number of Attributes: "<<meshInfo.num_attrs<<"\n"<<std::flush;
   for(int attr_I=0;attr_I<meshInfo.num_attrs;++attr_I)
      {
      char attrName[100];
      char attrSpaceString[100];
      char attrTypeString[100];
      hsize_t sdim[64];
      hid_t attrID=H5Aopen_by_idx(meshFile,".",H5_INDEX_CRT_ORDER,H5_ITER_INC,(hsize_t)attr_I,H5P_DEFAULT,H5P_DEFAULT);
      H5Aget_name(attrID,100,attrName);
      hid_t attrSpace=H5Aget_space(attrID);
      hid_t attrType=H5Aget_type(attrID);
      int rank=H5Sget_simple_extent_ndims(attrSpace);
      herr_t ret=H5Sget_simple_extent_dims(attrSpace,sdim,NULL);

      std::cout<<"------Attribute: "<<attrName<<"\n"<<std::flush;
      std::cout<<"-----------Rank: "<<rank<<"\n"<<std::flush;
      std::cout<<"------Dimension: "<<std::flush;
      for(int dim_I=0;dim_I<rank;++dim_I)
         {
         std::cout<<(int)sdim[dim_I]<<std::flush;
         }
      std::cout<<"\n"<<std::flush;

      if(H5T_FLOAT==H5Tget_class(attrType))
         {
         std::cout<<"-----------Type: H5T_FLOAT\n"<<std::flush;
         size_t npoints=H5Sget_simple_extent_npoints(attrSpace);
         float* float_array=(float *)malloc(sizeof(float)*(int)npoints);
         ret=H5Aread(attrID,attrType,float_array);
         std::cout<<"---------Values: "<<std::flush;
         for(int point_I=0;point_I<(int)npoints;++point_I)
            std::cout<<float_array[point_I]<<std::flush;
         std::cout<<"\n"<<std::flush;
         free(float_array);
         }
      else if(H5T_INTEGER==H5Tget_class(attrType))
         {
         std::cout<<"-----------Type: H5T_INTEGER\n"<<std::flush;
         size_t npoints=H5Sget_simple_extent_npoints(attrSpace);
         int* int_array=(int *)malloc(sizeof(int)*(int)npoints);
         ret=H5Aread(attrID,attrType,int_array);
         std::cout<<"---------Values: "<<std::flush;
         for(int point_I=0;point_I<(int)npoints;++point_I)
            std::cout<<int_array[point_I]<<std::flush;
         std::cout<<"\n"<<std::flush;
         free(int_array);
         }
      H5Tclose(attrType);
      }
   H5Gclose(meshGroupID);

   /* Get the connectivity from the mesh h5 file: */
   std::cout<<"Loading Connectivity...\n"<<std::flush;
   #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
   hid_t connDataSet=H5Dopen(meshFile,"/connectivity");
   #else
   hid_t connDataSet=H5Dopen(meshFile,"/connectivity",H5P_DEFAULT);
   #endif
   /* Get the filespace handle: */
   hid_t connFileSpace=H5Dget_space(connDataSet); 

   /* Get connectivity datatype: */
   hid_t connDataType=H5Dget_type(connDataSet);
   H5T_class_t connClass=H5Tget_class(connDataType);
   char connClassString[100];
   switch(connClass)
      {
      case H5T_INTEGER:
         strcpy(connClassString,"H5T_INTEGER");
         break;
      case H5T_FLOAT:
         strcpy(connClassString,"H5T_FLOAT");
         break;
      case H5T_STRING:
         strcpy(connClassString,"H5T_STRING");
         break;
      }
   std::cout<<"---Type: "<<connClassString<<"\n"<<std::flush;
   H5T_order_t connOrder=H5Tget_order(connDataType);
   char connOrderString[100];
   switch(connOrder)
      {
      case H5T_ORDER_LE:
         strcpy(connOrderString,"H5T_ORDER_LE");
         break;
      case H5T_ORDER_BE:
         strcpy(connOrderString,"H5T_ORDER_BE");
         break;
      }
   std::cout<<"---Order: "<<connOrderString<<"\n"<<std::flush;
   size_t connSize=H5Tget_size(connDataType);
   std::cout<<"---Size: "<<connSize<<"\n"<<std::flush;

   /* Get the vertices from the mesh h5 file: */
   #if (H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Dopen_vers == 1
   hid_t vertDataSet=H5Dopen(meshFile,"/vertices");
   #else 
   hid_t vertDataSet=H5Dopen(meshFile,"/vertices",H5P_DEFAULT);
   #endif
   /* Get the filespace handle: */ 
   hid_t vertFileSpace=H5Dget_space(vertDataSet);

   /* Close all handles: */
   H5Dclose(connDataSet);
   H5Sclose(connFileSpace);
   H5Dclose(vertDataSet);
   H5Sclose(vertFileSpace);
   
   /*H5Dread(meshDataSet,H5T_NATIVE_INT,H5S_ALL,H5S_ALL,H5P_DEFAULT,meshSize);*/

   /*std::cout<<"Resolution: "<<meshSize<<"\n"<<std::flush;*/

   /* Return the result data set: */
	return result;
	}

Visualization::Abstract::DataSetRenderer* UnderworldHDF5File::getRenderer(const Visualization::Abstract::DataSet* dataSet) const
	{
	return new EarthDataSetRenderer<DataSet,DataSetRenderer>(dataSet);
	}

int UnderworldHDF5File::getNumScalarAlgorithms(void) const
	{
	return BaseModule::getNumScalarAlgorithms();
	}

int UnderworldHDF5File::getNumVectorAlgorithms(void) const
	{
	return 0;
	}

}

}

/***************************
Plug-in interface functions:
***************************/

extern "C" Visualization::Abstract::Module* createFactory(Plugins::FactoryManager<Visualization::Abstract::Module>& manager)
	{
	/* Create module object and insert it into class hierarchy: */
	Visualization::Concrete::UnderworldHDF5File* module=new Visualization::Concrete::UnderworldHDF5File();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
