/***********************************************************************
Sasha3DFile - Class defining lowest-common-denominator ASCII
file format for curvilinear grids in Cartesian or spherical coordinates.
Vertex positions and attributes are stored in separate files.
Copyright (c) 2008 Oliver Kreylos

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
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <Misc/SelfDestructPointer.h>
#include <Misc/ThrowStdErr.h>
#include <Misc/File.h>
#include <Plugins/FactoryManager.h>
#include <Math/Math.h>

#include <Concrete/Sasha3DFile.h>

namespace Visualization {

namespace Concrete {

/************************************
Methods of class Sasha3DFile:
************************************/

Sasha3DFile::Sasha3DFile(void)
   :BaseModule("Sasha3DFile")
   {
   }

Visualization::Abstract::DataSet* Sasha3DFile::load(const std::vector<std::string>& args,Cluster::MulticastPipe* pipe) const
   {
   /* Create the result data set: */
   Misc::SelfDestructPointer<DataSet> result(new DataSet);

   /* Parse command line parameters related to the grid definition file: */
   std::vector<std::string>::const_iterator argIt=args.begin();
   
   /* Open the grid definition file: */
   std::cout<<"Reading file "<<*argIt<<"..."<<std::flush;
   Misc::File gridFile(argIt->c_str(),"rt");
   
   /* Parse the grid file header: */
   DS::Index numVertices(-1,-1,-1);
   unsigned int lineIndex = 0;
   char line[256];
   
   /* Skip the header line: */
   gridFile.gets(line,sizeof(line));

   /* Check if the file is already over: */
   if(gridFile.eof())
      Misc::throwStdErr("Sasha3DFile::load: early end-of-file in grid file %s",argIt->c_str());
      
   /* Read the next line from the file: */
   gridFile.gets(line,sizeof(line));
      
   /* Read the grid size: */
   if(sscanf(line+2,"%d  %d  %d",&numVertices[0],&numVertices[1],&numVertices[2])!=3)
      Misc::throwStdErr("Sasha3DFile::load: invalid grid size in line %u in grid file %s",lineIndex,argIt->c_str());
         
   /* Initialize the data set: */
   DS& dataSet=result->getDs();
   dataSet.setGrid(numVertices);
   dataSet.addSlice(); // Add a single scalar variable to the data set's grid
   
   /* Initialize the result data set's data value: */
   DataValue& dataValue = result->getDataValue();
   dataValue.initialize(&dataSet,0);
   dataValue.setScalarVariableName(0,"Resistivity"); // Set the name of the first scalar variable

   /* Read all cell offsets: */
   std::cout<<"   0%"<<std::flush;
   DS::Index index(0);

   /* Storage for the nodes and data: */
   std::vector<double> offsets[3];
   std::vector<double> resistivity;
   double offsetCumSums[3];
   //std::vector<double>::iterator offsetIterators[3];

   for(unsigned xyz_I=0;xyz_I<3;++xyz_I)
      {
      unsigned charIndex = 0;
      char valueBuffer[40];

      while(index[xyz_I]<numVertices[xyz_I]&&!gridFile.eof())
         {
         /* Read the next line: */
         char nextChar;
         bool readRealValue=false;
         double realValue;

         nextChar=gridFile.getc();
         if(isspace(nextChar)||nextChar=='\n')
            {
            if(charIndex>0)
               {
               sscanf(valueBuffer,"%lf",&realValue);
               offsets[xyz_I].push_back(realValue);
               offsetCumSums[xyz_I]+=(realValue);
               ++index[xyz_I];
               }
            readRealValue=false;
            charIndex = 0;
            continue;
            }
         else
            {
            readRealValue=true;
            }
         if(readRealValue)
            {
            valueBuffer[charIndex]=nextChar;
            ++charIndex;
            }
         }
      }
   
   //double startX=-*std::max_element(offsets[0].begin(),offsets[0].end());
   //double startY=-*std::max_element(offsets[1].begin(),offsets[1].end());
   //double startZ=*std::max_element(offsets[2].begin(),offsets[2].end());

   double startX=0.0;
   double startY=0.0;
   double startZ=0.0;

   std::cout<<"Max X: "<<startX<<"\n"<<std::flush;
   std::cout<<"Max Y: "<<startY<<"\n"<<std::flush;
   std::cout<<"Max Z: "<<startZ<<"\n"<<std::flush;
   std::cout<<"Cumulative Sum:\n"<<std::flush;
   std::cout<<"X: "<<offsetCumSums[0]<<"\n"<<std::flush;
   std::cout<<"Y: "<<offsetCumSums[1]<<"\n"<<std::flush;
   std::cout<<"Z: "<<offsetCumSums[2]<<"\n"<<std::flush;

   //std::sort(offsets[0].begin(),offsets[0].end());
   //std::sort(offsets[1].begin(),offsets[1].end());
   //std::sort(offsets[2].begin(),offsets[2].end());
   
   /*
   int k_counter=0;
   for(offsetIterators[0]=offsets[0].begin();offsetIterators[0]!=offsets[0].end();++offsetIterators[0])
      {
      std::cout<<k_counter<<": "<<*offsetIterators[0]<<"\n"<<std::flush;
      ++k_counter;
      }

   k_counter=0;
   for(offsetIterators[1]=offsets[1].begin();offsetIterators[1]!=offsets[1].end();++offsetIterators[1])
      {
      std::cout<<k_counter<<": "<<*offsetIterators[1]<<"\n"<<std::flush;
      ++k_counter;
      }

   k_counter=0;
   for(offsetIterators[2]=offsets[2].begin();offsetIterators[2]!=offsets[2].end();++offsetIterators[2])
      {
      std::cout<<k_counter<<": "<<*offsetIterators[2]<<"\n"<<std::flush;
      ++k_counter;
      }
   */

   int numOfNodes=numVertices[0]*numVertices[1]*numVertices[2];
   for(int data_I=0;data_I<numOfNodes&&!gridFile.eof();++data_I)
      {
      double realValue;
      /* Read data values: */
      gridFile.gets(line,sizeof(line));
      sscanf(line,"%lf",&realValue);
      resistivity.push_back(realValue);
      }

   /* Add the vertices and vertex data into the dataset: */
   DS::Index coordIndex;
   std::ofstream sashaFile("Sasha.txt");
   std::ofstream resistivityFile("Sasha-resistivity.txt");
   std::ofstream newCoord("New-Coord.txt");
   int counter=0;
   double pos[3]={startX,startY,startZ};
   for(coordIndex[0]=0;coordIndex[0]<numVertices[0];++coordIndex[0])
      {
      pos[1]=startY;
      for(coordIndex[1]=0;coordIndex[1]<numVertices[1];++coordIndex[1])
         {
         pos[0]=startX;
         for(coordIndex[2]=0;coordIndex[2]<numVertices[2];++coordIndex[2])
            {
            double value;
            value = resistivity[counter];
            sashaFile<<pos[0]<<" "<<pos[1]<<" "<<pos[2]<<"\n"<<std::flush;
            resistivityFile<<value<<"\n"<<std::flush;
            /* Store the position and value in the data set: */
            newCoord<<pos[0]<<" "<<pos[1]<<" "<<pos[2]<<"\n"<<std::flush;
            dataSet.getVertexPosition(coordIndex)=DS::Point(pos);
            dataSet.getVertexValue(0,coordIndex)=Scalar(Math::log10(value));
            pos[0]+=offsets[2][coordIndex[2]];
            ++counter;
            }
         pos[1]+=offsets[1][coordIndex[1]];
         }
      pos[2]+=offsets[0][coordIndex[0]];
      }
   newCoord.close();
   sashaFile.close();
   resistivityFile.close();

   /* Finalize the grid structure: */
   std::cout<<"Finalizing grid structure..."<<std::flush;
   dataSet.finalizeGrid();
   std::cout<<" done"<<std::endl;
   
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
   Visualization::Concrete::Sasha3DFile* module = new Visualization::Concrete::Sasha3DFile();
   
   /* Return module object: */
   return module;
   }

extern "C" void destroyFactory(Visualization::Abstract::Module* module)
   {
   delete module;
   }
