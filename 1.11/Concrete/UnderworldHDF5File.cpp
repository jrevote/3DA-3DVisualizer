/***********************************************************************
UnderworldHDF5File - Class to read Underworld unstructured mesh data
and its field variables (scalar, vector) in HDF5 format.

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
#include <Misc/SelfDestructPointer.h>
#include <Plugins/FactoryManager.h>

#include <Concrete/UnderworldHDF5File.h>
#include <hdf5.h>
#include <ctime>

/* X Y Z Mag: */
#define VECTOR_COMPONENT_COUNT 4

namespace Visualization {

namespace Concrete {

namespace {

/**********************
Helper data structures:
***********************/
enum FieldType 
   {
   SCALAR,
   VECTOR
   };

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
      default:
         return "UNKNOWN";
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
      default:
         return "UNKNOWN";
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
      default:
         return 0;
      }
   }

void readMetaDataFromH5(
   hid_t dataSet,
   hid_t &dataType,
   H5T_class_t &dataClass,
   hid_t &dataSpace,
   int &dataRank,
   hsize_t dataDims[])
   {
   /* Get DATATYPE: */
   dataType=H5Dget_type(dataSet);
   /* Get class: */
   dataClass=H5Tget_class(dataType);
   /* Get DATASPACE: */
   dataSpace=H5Dget_space(dataSet);
   /* Get rank: */
   dataRank=H5Sget_simple_extent_ndims(dataSpace);
   H5Sget_simple_extent_dims(dataSpace,dataDims,NULL);
       
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

void readRealDataFromH5(hid_t dataSet,int dataRank,hsize_t dataDims[],hid_t dataSpace,int* dataValues)
   {
   hsize_t dataStart[2],dataNodeCount[2];
   dataStart[1]=(hsize_t)0;
   dataNodeCount[0]=(hsize_t)1;
   dataNodeCount[1]=(hsize_t)dataDims[1];

   /* Create simple memory space for one record on the dataset: */
   hid_t dataMemSpace=H5Screate_simple(dataRank,dataNodeCount,NULL);
   int* dataBuffer=new int[dataDims[1]];

   /* For each record, read and store it in the values data structure: */
   for(unsigned data_I=0;data_I<dataDims[0];++data_I)
      {
      dataStart[0]=(hsize_t)data_I;
      H5Sselect_hyperslab(dataSpace,H5S_SELECT_SET,dataStart,NULL,dataNodeCount,NULL);
      H5Sselect_all(dataMemSpace);
      /* Read one record (defined by the memory space) and save in the buffer: */
      H5Dread(dataSet,H5T_NATIVE_INT,dataMemSpace,dataSpace,H5P_DEFAULT,dataBuffer);
      /* Pass the value from the buffer into the value data structure: */
      for(unsigned data_J=0;data_J<dataDims[1];++data_J)
         dataValues[(data_I*dataDims[1])+data_J]=dataBuffer[data_J];
      }

   /* Free temporary buffer: */
   delete[] dataBuffer; 
   /* Close temporary memory space: */
   H5Sclose(dataMemSpace);
   }

void readRealDataFromH5(hid_t dataSet,int dataRank,hsize_t dataDims[],hid_t dataSpace,double* dataValues)
   {
   hsize_t dataStart[2],dataNodeCount[2];
   dataStart[1]=(hsize_t)0;
   dataNodeCount[0]=(hsize_t)1;
   dataNodeCount[1]=(hsize_t)dataDims[1];

   /* Create simple memory space for one record on the dataset: */
   hid_t dataMemSpace=H5Screate_simple(dataRank,dataNodeCount,NULL);
   double* dataBuffer=new double[dataDims[1]];

   /* For each record, read and store it in the values data structure: */
   for(unsigned data_I=0;data_I<dataDims[0];++data_I)
      {
      dataStart[0]=(hsize_t)data_I;
      H5Sselect_hyperslab(dataSpace,H5S_SELECT_SET,dataStart,NULL,dataNodeCount,NULL);
      H5Sselect_all(dataMemSpace);
      /* Read one record (defined by the memory space) and save in the buffer: */
      H5Dread(dataSet,H5T_NATIVE_DOUBLE,dataMemSpace,dataSpace,H5P_DEFAULT,dataBuffer);
      /* Pass the value from the buffer into the value data structure: */
      for(unsigned data_J=0;data_J<dataDims[1];++data_J)
         dataValues[(data_I*dataDims[1])+data_J]=dataBuffer[data_J];
      }

   /* Free temporary buffer: */
   delete[] dataBuffer; 
   /* Close temporary memory space: */
   H5Sclose(dataMemSpace);
   }

void getFieldColumnCount(const char* fieldFileName,int& numFields,hsize_t vertDims)
   {
   hid_t fieldFile=H5Fopen(fieldFileName,H5F_ACC_RDONLY,H5P_DEFAULT);
   if(fieldFile<0)
      Misc::throwStdErr("UnderworldHDF5File::load: Invalid field file (%s) provided.",fieldFileName);

   hid_t fieldDataSet=H5Dopen2(fieldFile,"/data",H5P_DEFAULT);
   hid_t fieldSpace=H5Dget_space(fieldDataSet);
   hsize_t fieldDims[64];
   H5Sget_simple_extent_dims(fieldSpace,fieldDims,NULL);
   /* Get total column for the field variable and add it to the total: */
   int offset=fieldDims[1]-vertDims;
   numFields+=offset>0?offset:1;

   /* Close all handles: */
   H5Sclose(fieldSpace);
   H5Dclose(fieldDataSet);
   H5Fclose(fieldFile);
   }

void readFieldValues(
   DS& dataSet,
   DataValue& dataValue,
   std::vector<std::string> fieldFileNames,
   hsize_t vertDims[],
   int* sliceIndices,
   DS::VertexIndex* vertexIndices,
   FieldType fieldType)
   {
   int sliceOffset=0;

   /* Retrieve field values for each file: */
   for(unsigned field_I=0;field_I<fieldFileNames.size();++field_I)
      {
      const char* fieldFileName=fieldFileNames[field_I].c_str();
      std::cout<<"Loading values from: \""<<fieldFileName<<"\"...\n"<<std::flush;
      hid_t fieldFile=H5Fopen(fieldFileName,H5F_ACC_RDONLY,H5P_DEFAULT);

      if(fieldFile<0)
         Misc::throwStdErr("UnderworldHDF5File::load: Invalid field file provided.");
      hid_t fieldDataSet=H5Dopen2(fieldFile,"/data",H5P_DEFAULT);
      hid_t fieldDataType;
      H5T_class_t fieldClass;
      hid_t fieldSpace;
      int fieldRank;
      hsize_t fieldDims[64];
      readMetaDataFromH5(fieldDataSet,fieldDataType,fieldClass,fieldSpace,fieldRank,fieldDims);

      char tempName[100],fieldName[100];
      strcpy(tempName,fieldFileName);
      char* baseName=strtok(tempName,".");

      int offset=fieldDims[1]-vertDims[1];
      /* This check is required to determine if the vertex positions are defined in both the mesh
      and the field file or just the mesh file: */
      int columns=offset>0?offset:fieldDims[1];
      int start=offset>0?vertDims[1]:0;

      /* Add variable names: */
      switch(fieldType)
         {
         case SCALAR:
            if(offset>1)
               for(int field_J=0;field_J<offset;++field_J)
                  {
                  sprintf(fieldName,"%s-Component-%d",baseName,field_J);
                  dataValue.addScalarVariable(fieldName); 
                  }
            else
               {
               char* fieldName;
               fieldName=strtok(tempName,".");
               /* Add a new scalar variable: */
               dataValue.addScalarVariable(fieldName); 
               }
            break;
         case VECTOR:
            std::vector<std::string> componentNames;
            componentNames.push_back("X");
            componentNames.push_back("Y");
            componentNames.push_back("Z");
            componentNames.push_back("Magnitude");
            char fieldComponentName[100];
            /* Add a new vector variable: */
            int vectorVariableIndex=dataValue.addVectorVariable(baseName);
            for(unsigned field_J=0;field_J<componentNames.size();++field_J)
               {
               sprintf(fieldComponentName,"%s-%s",baseName,componentNames[field_J].c_str());
               dataValue.setVectorVariableScalarIndex(vectorVariableIndex,field_J,dataValue.addScalarVariable(fieldComponentName));
               }
            break;
         }

      hsize_t fieldStart[2],fieldNodeCount[2];
      fieldStart[1]=(hsize_t)0;
      fieldNodeCount[0]=(hsize_t)1; 
      fieldNodeCount[1]=(hsize_t)fieldDims[1]; 

      /* Create simple memory space for one record on the dataset: */
      hid_t fieldMemSpace=H5Screate_simple(fieldRank,fieldNodeCount,NULL);
      double* fieldBuffer=new double[fieldDims[1]];

      /* For each record, read and store it in the values data structure: */
      for(unsigned field_J=0;field_J<fieldDims[0];++field_J)
         {
         fieldStart[0]=(hsize_t)field_J;
         H5Sselect_hyperslab(fieldSpace,H5S_SELECT_SET,fieldStart,NULL,fieldNodeCount,NULL);
         H5Sselect_all(fieldMemSpace);
         DataValue::VVector vector;

         /* Read one record (defined by the memory space) and save in the buffer: */
         H5Dread(fieldDataSet,H5T_NATIVE_DOUBLE,fieldMemSpace,fieldSpace,H5P_DEFAULT,fieldBuffer);
         for(int field_K=0;field_K<columns;++field_K)
            {
            /* Assign field value to vertex in the dataset: */
            switch(fieldType)
               {
               case SCALAR:
                  dataSet.setVertexValue(sliceIndices[sliceOffset+field_K],vertexIndices[field_J],DS::ValueScalar(fieldBuffer[start+field_K]));
                  break;
               case VECTOR:
                  vector[field_K]=DS::ValueScalar(fieldBuffer[start+field_K]);
                  dataSet.setVertexValue(sliceIndices[sliceOffset*VECTOR_COMPONENT_COUNT+field_K],vertexIndices[field_J],vector[field_K]);
                  break;
               } 
            }
         if(fieldType==VECTOR)
            dataSet.setVertexValue(sliceIndices[field_I*VECTOR_COMPONENT_COUNT+3],vertexIndices[field_J],vector.mag());
         }
      if(offset>1)
         sliceOffset+=offset;

      /* Free temporary buffer: */
      delete[] fieldBuffer; 
      /* Close temporary memory space: */
      H5Sclose(fieldMemSpace);

      /* Close all handles: */
      H5Sclose(fieldSpace);
      H5Dclose(fieldDataSet);
      H5Fclose(fieldFile);
      }
   }

}

/***********************************
Methods of class UnderworldHDF5File:
************************************/

UnderworldHDF5File::UnderworldHDF5File(void)
	:BaseModule("UnderworldHDF5File")
	{
	}

Visualization::Abstract::DataSet* UnderworldHDF5File::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
	{
   /* Create result data set: */
   Misc::SelfDestructPointer<DataSet> result(new DataSet);
   DS& dataSet=result->getDs();

   /* Initialize the result data set's data value: */
   DataValue& dataValue=result->getDataValue();
   dataValue.initialize(&dataSet,0);

   /* Parse command line arguments: */
   const char* meshFileName=0;
   std::vector<std::string> scalarFileNames;
   std::vector<std::string> vectorFileNames;
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
         {
         nextScalar=true;
         nextVector=false;
         }
      /* Check if there are vector variables given: */
      else if(strcasecmp(argIt->c_str(),"-vector")==0)
         {
         nextScalar=false;
         nextVector=true;
         }
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
   std::cout<<"Reading Mesh...\n"<<std::flush;
   hid_t meshFile=H5Fopen(meshFileName,H5F_ACC_RDONLY,H5P_DEFAULT);
   /* Make sure that the mesh file is valid: */
   if(meshFile<0)
      Misc::throwStdErr("UnderworldHDF5File::load: Invalid mesh file provided.");
   #if(H5_VERS_MAJOR == 1 && H5_VERS_MINOR < 8) || H5Gopen_vers == 1
   hid_t meshGroupID=H5Gopen(meshFile,"/");
   #else
   hid_t meshGroupID=H5Gopen(meshFile,"/",H5P_DEFAULT);
   #endif
   H5O_info_t meshInfo;
   /* Retrieve standard information from the mesh: */
   H5Oget_info(meshGroupID,&meshInfo);
   std::cout<<"---Number of Attributes: "<<meshInfo.num_attrs<<"\n"<<std::flush;
   int* meshResolution = NULL;

   /* Iterate through the mesh attributes: */
   for(unsigned attr_I=0;attr_I<(unsigned)meshInfo.num_attrs;++attr_I)
      {
      char attrName[100];
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
      H5Sget_simple_extent_dims(attrSpace,attrDim,NULL);
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
         H5Aread(attrID,attrTypeMem,float_array);
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
         H5Aread(attrID,attrTypeMem,int_array);
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
   double* vertValues=new double[vertDims[0]*vertDims[1]];
   readRealDataFromH5(vertDataSet,vertRank,vertDims,vertSpace,vertValues);

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
   int* connValues=new int[connDims[0]*connDims[1]];
   readRealDataFromH5(connDataSet,connRank,connDims,connSpace,connValues);

   /* Get scalar values from each of the scalar field files: */
   int numScalars=0;
   for(int scalar_I=0;scalar_I<int(scalarFileNames.size());++scalar_I)
      getFieldColumnCount(scalarFileNames[scalar_I].c_str(),numScalars,vertDims[1]);
   int* scalarSliceIndices=new int[numScalars];
   for(int scalar_I=0;scalar_I<numScalars;++scalar_I)
      scalarSliceIndices[scalar_I]=dataSet.addSlice();

   /* Get vector values from each of the vector field files: */
   int numVectors=int(vectorFileNames.size());
   for(int vector_I=0;vector_I<int(vectorFileNames.size());++vector_I)
      getFieldColumnCount(vectorFileNames[vector_I].c_str(),numVectors,vertDims[1]);
   int* vectorSliceIndices=new int[numVectors*VECTOR_COMPONENT_COUNT];
   for(int vector_I=0;vector_I<numVectors;++vector_I)
      {
      for(int vector_J=0;vector_J<3;++vector_J)
         vectorSliceIndices[vector_I*VECTOR_COMPONENT_COUNT+vector_J]=dataSet.addSlice();
      vectorSliceIndices[vector_I*VECTOR_COMPONENT_COUNT+3]=dataSet.addSlice();
      }

   /* Reserve space for dataSet: */
   dataSet.reserveVertices((size_t)vertDims[0]);
   dataSet.reserveCells((size_t)connDims[0]);

   /* Load all grid vertices into the dataset: */
   std::cout<<"---Loading Grid Vertices into 3DVisualizer...\n"<<std::flush;
   DS::VertexIndex* vertexIndices=new DS::VertexIndex[vertDims[0]];
   for(unsigned vert_I=0;vert_I<vertDims[0];++vert_I)
      {
      DS::Point vertexPosition;
      for(unsigned vert_J=0;vert_J<vertDims[1];++vert_J)
         vertexPosition[vert_J]=Scalar(vertValues[(vert_I*vertDims[1])+vert_J]);
      vertexIndices[vert_I]=dataSet.addVertex(vertexPosition).getIndex();
      }
   std::cout<<"------Number of vertices loaded: "<<dataSet.getTotalNumVertices()<<"\n"<<std::flush;

   /* Load all grid cells into the dataset: */
   std::cout<<"---Loading Grid Cells into 3DVisualizer...\n"<<std::flush;
   DS::VertexID* cellVertices=new DS::VertexID[8];
   static const int vertexOrder[8]={0,1,3,2,4,5,7,6}; // vertex ordering
   for(unsigned conn_I=0;conn_I<connDims[0];++conn_I)
      {
      for(unsigned conn_J=0;conn_J<8;++conn_J)
         cellVertices[vertexOrder[conn_J]]=DS::VertexID(connValues[(conn_I*connDims[1])+conn_J]);
      dataSet.addCell(cellVertices);
      }
   std::cout<<"------Number of cells loaded: "<<dataSet.getTotalNumCells()<<"\n"<<std::flush;

   std::cout<<"------Number of slices loaded: "<<dataSet.getNumSlices()<<"\n"<<std::flush;

   /* Finalize the grid structure: */
   std::cout<<"Finalizing Grid Structure..."<<std::flush;
   dataSet.finalizeGrid();
   std::cout<<" (DONE)"<<std::endl;

   /* Get scalar values from each of the scalar field files: */
   readFieldValues(dataSet,dataValue,scalarFileNames,vertDims,scalarSliceIndices,vertexIndices,SCALAR);

   /* Get vector values from each of the vector field files: */
   readFieldValues(dataSet,dataValue,vectorFileNames,vertDims,vectorSliceIndices,vertexIndices,VECTOR);
   
   /* Free used data structures: */
   delete[] vertexIndices;
   delete[] cellVertices;
   delete[] scalarSliceIndices;
   delete[] vectorSliceIndices;
   delete[] vertValues;
   delete[] connValues;

   /* Close all handles: */
   H5Dclose(connDataSet);
   H5Sclose(connSpace);
   H5Dclose(vertDataSet);
   H5Sclose(vertSpace);
   H5Fclose(meshFile);

   /* Return the result data set: */
	return result.releaseTarget();
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
