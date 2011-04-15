/***********************************************************************
UnderworldHDF5File - Class to read unstructured mesh data in NASA
Plot3D format.
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
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
***********************************************************************/

#include <stdio.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/UnderworldHDF5File.h>
#include <hdf5.h>

namespace Visualization {

namespace Concrete {

namespace {

/****************
Helper functions:
*****************/
const char* getClassString(H5T_class_t classID)
   {
   switch(classID)
      {
      case H5T_INTEGER:
         return "H5T_INTEGER";
      case H5T_FLOAT:
         return "H5T_FLOAT";
      case H5T_STRING:
         return "H5T_STRING";
      }
   }

const char* getOrderString(H5T_order_t orderID)
   {
   switch(orderID)
      {
      case H5T_ORDER_LE:
         return "H5T_ORDER_LE";
      case H5T_ORDER_BE:
         return "H5T_ORDER_BE";
      }
   }

const hid_t getNativeTypeFromDataClass(H5T_class_t dataClass)
   {
   switch(dataClass)
      {
      case H5T_FLOAT:
         return H5T_NATIVE_FLOAT;
      case H5T_INTEGER:
         return H5T_NATIVE_INT;
      }
   }

void readMetaDataFromH5(hid_t dataSet,hid_t &dataType,H5T_class_t &dataClass,hid_t &dataSpace,int &dataRank,hsize_t dataDims[])
   {
   /* Get DATATYPE: */
   dataType=H5Dget_type(dataSet);
   /* Get class: */
   dataClass=H5Tget_class(dataType);
   /* Get DATASPACE: */
   dataSpace=H5Dget_space(dataSet);
   /* Get rank: */
   dataRank=H5Sget_simple_extent_ndims(dataSpace);
   herr_t dataRet=H5Sget_simple_extent_dims(dataSpace,dataDims,NULL);
       
   /* Display data information: */
   std::cout<<"------Rank: "<<dataRank<<"\n"<<std::flush;
   std::cout<<"------Dimensions: "<<std::flush;
   for(int data_I=0;data_I<dataRank;++data_I)
      {
      std::cout<<dataDims[data_I]<<std::flush;
      if((int)dataRank>1&&(data_I+1)<(int)dataRank)
         std::cout<<" "<<std::flush;
      }
   std::cout<<"\n"<<std::flush;

   /* Check data DATATYPE: */
   char dataClassString[100];
   strcpy(dataClassString,getClassString(dataClass));
   std::cout<<"------Type: "<<dataClassString<<"\n"<<std::flush;

   /* Get dataices order: */
   H5T_order_t dataOrder=H5Tget_order(dataType);

   /* Check data order: */
   char dataOrderString[100];
   strcpy(dataOrderString,getOrderString(dataOrder));
   std::cout<<"------Order: "<<dataOrderString<<"\n"<<std::flush;

   /* Get data size: */ 
   size_t dataSize=H5Tget_size(dataType);
   std::cout<<"------Size: "<<dataSize<<"\n"<<std::flush;
   }
}

void readRealDataFromH5(int dataRank,hsize_t dataDims[],hid_t dataSpace,char* type)
   {
   }

/***************************************
Methods of class UnderworldHDF5File:
***************************************/

UnderworldHDF5File::UnderworldHDF5File(void)
	:BaseModule("UnderworldHDF5File")
	{
	}

Visualization::Abstract::DataSet* UnderworldHDF5File::load(const std::vector<std::string>& args,Comm::MulticastPipe* pipe) const
	{
   /* Create result data set: */
   DataSet* result=new DataSet;
   UnderworldHDF5File::DS* dataSet=&result->getDs();

   /* Parse command line arguments: */
   const char* meshFileName=0;
   std::vector<std::string> scalarFileNames;
   std::vector<std::string> vectorFileNames;
   bool nextMesh=false;
   bool nextScalar=false;
   bool nextVector=false;
   /* Iterate the command line arguments given: */
   for(std::vector<std::string>::const_iterator argIt=args.begin();argIt!=args.end();++argIt)
      {
      /* Check for the mesh file: */
      if(strcasecmp(argIt->c_str(),"-mesh")==0)
         {
         ++argIt;
         meshFileName=argIt->c_str();
         std::cout<<"Received Mesh file: \""<<meshFileName<<"\"\n"<<std::flush;
         }
      /* Check if there are scalar variables given: */
      else if(strcasecmp(argIt->c_str(),"-scalar")==0)
         nextScalar=true;
      /* Check if there are vector variables given: */
      else if(strcasecmp(argIt->c_str(),"-vector")==0)
         nextVector=true;
      else
         {
         if(nextScalar)
            {
            std::cout<<"Received Scalar file: \""<<argIt->c_str()<<"\"\n"<<std::flush;
            /* Save all scalar variable filenames into the list: */
            scalarFileNames.push_back(*argIt);
            }
         else if(nextVector)
            {
            std::cout<<"Received Vector file: \""<<argIt->c_str()<<"\"\n"<<std::flush;
            /* Save all vector variable filanames into the list: */
            vectorFileNames.push_back(*argIt);
            }
         }
      }

   /* Make sure that a mesh file is given: */
   if(meshFileName==0)
      Misc::throwStdErr("UnderworldHDF5File::load: No input mesh name provided.");

   /* Open/read the mesh dataset: */
   int meshSize[3];
   std::cout<<"Reading Mesh...\n"<<std::flush;
   hid_t meshFile=H5Fopen(meshFileName,H5F_ACC_RDONLY,H5P_DEFAULT);
   /* Make sure that the mesh file is valid: */
   if(meshFile<0)
      Misc::throwStdErr("UnderworldHDF5File::load: Invalid mesh file provided.");
   hid_t meshGroupID=H5Gopen(meshFile,"/");
   H5O_info_t meshInfo;
   /* Retrieve standard information from the mesh: */
   H5Oget_info(meshGroupID,&meshInfo);
   std::cout<<"---Number of Attributes: "<<meshInfo.num_attrs<<"\n"<<std::flush;
   int* meshResolution;

   /* Iterate through the mesh attributes: */
   for(int attr_I=0;attr_I<(unsigned)meshInfo.num_attrs;++attr_I)
      {
      char attrName[100];
      char attrSpaceString[100];
      char attrTypeString[100];
      hsize_t attrDim[64];

      /* Open the attribute using the attribute index: */
      hid_t attrID=H5Aopen_by_idx(meshFile,".",H5_INDEX_CRT_ORDER,H5_ITER_INC,(hsize_t)attr_I,H5P_DEFAULT,H5P_DEFAULT);
      /* Get attribute DATATYPE: */
      hid_t attrType=H5Aget_type(attrID);
      /* Get attribute class: */
      H5T_class_t attrClass=H5Tget_class(attrType);
      /* Get attribute DATASPACE: */
      hid_t attrSpace=H5Aget_space(attrID);
      /* Get attribute name: */
      H5Aget_name(attrID,100,attrName);
      /* Get attribute rank: */
      int attrRank=H5Sget_simple_extent_ndims(attrSpace);
      /* Get attribute dimension: */
      herr_t attrRet=H5Sget_simple_extent_dims(attrSpace,attrDim,NULL);
      /* Get attribute DATATYPE native memory: */
      hid_t attrTypeMem=H5Tget_native_type(attrType,H5T_DIR_ASCEND);

      /* Display attribute information: */
      std::cout<<"------Attribute: "<<"\""<<attrName<<"\"\n"<<std::flush;
      std::cout<<"---------Rank: "<<attrRank<<"\n"<<std::flush;
      std::cout<<"---------Dimension: "<<std::flush;
      bool currMeshAttr=false;
      if(!strcasecmp(attrName,"mesh resolution"))
         {
         meshResolution=new int[attrDim[0]];
         currMeshAttr=true;
         }
      for(int dim_I=0;dim_I<attrRank;++dim_I)
         {
         std::cout<<(int)attrDim[dim_I]<<std::flush;
         }
      std::cout<<"\n"<<std::flush;

      /* Based on the DATATYPE, read the attribute values into the buffer: */
      if(attrClass==H5T_FLOAT)
         {
         std::cout<<"---------Type: H5T_FLOAT\n"<<std::flush;
         size_t npoints=H5Sget_simple_extent_npoints(attrSpace);
         float* float_array=new float[npoints];
         /* Read the attribute values: */
         attrRet=H5Aread(attrID,attrTypeMem,float_array);
         std::cout<<"---------Values: "<<std::flush;
         for(int value_I=0;value_I<(int)npoints;++value_I)
            {
            std::cout<<float_array[value_I]<<std::flush;
            if((int)npoints>1&&(value_I+1)<(int)npoints)
               std::cout<<" "<<std::flush;
            }
         std::cout<<"\n"<<std::flush;
         delete[] float_array;
         }
      else if(attrClass==H5T_INTEGER)
         {
         std::cout<<"---------Type: H5T_INTEGER\n"<<std::flush;
         size_t npoints=H5Sget_simple_extent_npoints(attrSpace);
         int* int_array=new int[npoints];
         /* Read the attribute values: */
         attrRet=H5Aread(attrID,attrTypeMem,int_array);
         std::cout<<"---------Values: "<<std::flush;
         for(int value_I=0;value_I<(int)npoints;++value_I)
            {
            std::cout<<int_array[value_I]<<std::flush;
            if((int)npoints>1&&(value_I+1)<(int)npoints)
               std::cout<<" "<<std::flush;
            if(currMeshAttr)
               meshResolution[value_I]=(int)int_array[value_I];
            }
         std::cout<<"\n"<<std::flush;
         delete[] int_array;
         }
      /* Close handles: */
      H5Tclose(attrType);
      H5Sclose(attrSpace);
      }
   /* Close mesh: */
   H5Gclose(meshGroupID);

   /* Get the vertices from the mesh h5 file: */
   std::cout<<"---Loading Vertices...\n"<<std::flush;
   hid_t vertDataSet=H5Dopen2(meshFile,"/vertices",H5P_DEFAULT);
   hid_t vertDataType;
   H5T_class_t vertClass;
   hid_t vertSpace;
   int vertRank;
   hsize_t vertDims[64];
   readMetaDataFromH5(vertDataSet,vertDataType,vertClass,vertSpace,vertRank,vertDims);

   /* Read vertices values: */
   herr_t vertRet;
   float* vertValues=new float[vertDims[0]*vertDims[1]];
   hsize_t vertStart[2],vertNodeCount[2];
   vertStart[1]=(hsize_t)0;
   vertNodeCount[0]=(hsize_t)1;
   vertNodeCount[1]=(hsize_t)vertDims[1];
   /* Create simple memory space for one record on the dataset: */
   hid_t vertMemSpace=H5Screate_simple(vertRank,vertNodeCount,NULL);
   float* vertBuffer=new float[vertDims[1]];
   /* For each record, read and store it in the values data structure: */
   for(int vert_I=0;vert_I<vertDims[0];++vert_I)
      {
      vertStart[0]=(hsize_t)vert_I;
      H5Sselect_hyperslab(vertSpace,H5S_SELECT_SET,vertStart,NULL,vertNodeCount,NULL);
      H5Sselect_all(vertMemSpace);
      /* Read one record (defined by the memory space) and save in the buffer: */
      vertRet=H5Dread(vertDataSet,H5T_NATIVE_FLOAT,vertMemSpace,vertSpace,H5P_DEFAULT,vertBuffer);
      /* Pass the value from the buffer into the value data structure: */
      for(int vert_J=0;vert_J<vertDims[1];++vert_J)
         vertValues[(vert_I*vertDims[1])+vert_J]=vertBuffer[vert_J];
      }
   /* Free temporary buffer: */
   delete[] vertBuffer; 
   /* Close temporary memory space: */
   H5Sclose(vertMemSpace);

   /* Load all grid vertices into the dataset: */
   std::cout<<"---Loading Grid Vertices into 3DVisualizer...\n"<<std::flush;
   UnderworldHDF5File::DS::GridVertexIterator* vertices=new UnderworldHDF5File::DS::GridVertexIterator[vertDims[0]];
   for(int vert_I=0;vert_I<vertDims[0];++vert_I)
      vertices[vert_I]=dataSet->addVertex(UnderworldHDF5File::DS::Point(),UnderworldHDF5File::DS::Value());
         
   for(int vert_I=0;vert_I<vertDims[0];++vert_I)
      {
      for(int vert_J=0;vert_J<vertDims[1];++vert_J)
         vertices[vert_I]->pos[vert_J]=vertValues[(vert_I*vertDims[1])+vert_J];
      }
   std::cout<<"------Total number of Vertices: "<<dataSet->getTotalNumVertices()<<"\n"<<std::flush;

   /* Get the connectivity from the mesh h5 file: */
   std::cout<<"---Loading Connectivity...\n"<<std::flush;
   hid_t connDataSet=H5Dopen2(meshFile,"/connectivity",H5P_DEFAULT);
   hid_t connDataType;
   H5T_class_t connClass;
   hid_t connSpace;
   int connRank;
   hsize_t connDims[64];
   readMetaDataFromH5(connDataSet,connDataType,connClass,connSpace,connRank,connDims);

   /* Read connectivity values: */
   herr_t connRet;
   int* connValues=new int[connDims[0]*connDims[1]];
   hsize_t connStart[2],connNodeCount[2];
   connStart[1]=(hsize_t)0;
   connNodeCount[0]=(hsize_t)1;
   connNodeCount[1]=(hsize_t)connDims[1];
   /* Create simple memory space for one record on the dataset: */
   hid_t connMemSpace=H5Screate_simple(connRank,connNodeCount,NULL);
   int* connBuffer=new int[connDims[1]];
   /* For each record, read and store it in the values data structure: */
   for(int conn_I=0;conn_I<connDims[0];++conn_I)
      {
      connStart[0]=(hsize_t)conn_I;
      H5Sselect_hyperslab(connSpace,H5S_SELECT_SET,connStart,NULL,connNodeCount,NULL);
      H5Sselect_all(connMemSpace);
      /* Read one record (defined by the memory space) and save it in the buffer: */
      connRet=H5Dread(connDataSet,H5T_NATIVE_INT,connMemSpace,connSpace,H5P_DEFAULT,connBuffer);
      /* Pass the value from the buffer into the value data structure: */
      for(int conn_J=0;conn_J<connDims[1];++conn_J)
         connValues[(conn_I*connDims[1])+conn_J]=connBuffer[conn_J];
      }
   delete[] connBuffer; 
   H5Sclose(connMemSpace);

   /* Load all grid cells into the dataset: */
   std::cout<<"---Loading Grid Cells into 3DVisualizer...\n"<<std::flush;
   for(int conn_I=0;conn_I<connDims[0];++conn_I)
      {
      UnderworldHDF5File::DS::GridVertexIterator* cellVertices=new UnderworldHDF5File::DS::GridVertexIterator[connDims[1]];
      for(int conn_J=0;conn_J<connDims[1];++conn_J)
         cellVertices[conn_J]=vertices[connValues[(conn_I*connDims[1])+conn_J]];
      /* Add the cell to the dataset: */
      dataSet->addCell(cellVertices);
      delete[] cellVertices;
      }
   delete[] vertices;
   std::cout<<"------Total number of Cells: "<<dataSet->getTotalNumCells()<<"\n"<<std::flush;

   /* Free used data structures: */
   delete[] vertValues;
   delete[] connValues;

   /* Close all handles: */
   H5Dclose(connDataSet);
   H5Sclose(connSpace);
   H5Dclose(vertDataSet);
   H5Sclose(vertSpace);
   H5Fclose(meshFile);

   /* Finalize the grid structure: */
   std::cout<<"Finalizing Grid Structure..."<<std::flush;
   dataSet->finalizeGrid();
   //result->getDs().finalizeGrid();
   std::cout<<" (DONE)"<<std::endl;
   
   /* Return the result data set: */
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
	Visualization::Concrete::UnderworldHDF5File* module=new Visualization::Concrete::UnderworldHDF5File();
	
	/* Return module object: */
	return module;
	}

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
	{
	delete module;
	}
