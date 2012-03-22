/***********************************************************************
VariableManager - Helper class to manage the scalar and vector variables
that can be extracted from a data set.
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

#include <string.h>
#include <stdio.h>
#include <stdexcept>
#include <Misc/CreateNumberedFileName.h>
#include <GL/GLColorMap.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <Vrui/Vrui.h>

#include <Abstract/ScalarExtractor.h>
#include <Abstract/VectorExtractor.h>

#include <ColorBar.h>
#include <ColorMap.h>

#include <Abstract/VariableManager.h>

namespace Visualization {

namespace Abstract {

/************************************************
Methods of class VariableManager::ScalarVariable:
************************************************/

VariableManager::ScalarVariable::ScalarVariable(void)
	:scalarExtractor(0),
	 colorMap(0),
	 palette(0)
	{
	}

VariableManager::ScalarVariable::~ScalarVariable(void)
	{
	delete scalarExtractor;
	delete colorMap;
	delete palette;
	}

/********************************
Methods of class VariableManager:
********************************/

void VariableManager::prepareScalarVariable(int scalarVariableIndex)
	{
	ScalarVariable& sv=scalarVariables[scalarVariableIndex];
	
	/* Get a new scalar extractor: */
	sv.scalarExtractor=dataSet->getScalarExtractor(scalarVariableIndex);
	
	/* Calculate the scalar extractor's value range: */
	sv.valueRange=dataSet->calcScalarValueRange(sv.scalarExtractor);
	
	/* Create a 256-entry OpenGL color map for rendering: */
	sv.colorMap=new GLColorMap(GLColorMap::GREYSCALE|GLColorMap::RAMP_ALPHA,1.0f,1.0f,sv.valueRange.first,sv.valueRange.second);
	}

void VariableManager::colorMapChangedCallback(Misc::CallbackData* cbData)
	{
	/* Export the changed palette to the current color map: */
	paletteEditor->exportColorMap(*scalarVariables[currentScalarVariableIndex].colorMap);
	
	Vrui::requestUpdate();
	}

void VariableManager::savePaletteCallback(Misc::CallbackData* cbData)
	{
	if(Vrui::isMaster())
		{
		try
			{
			char numberedFileName[40];
			paletteEditor->savePalette(Misc::createNumberedFileName("SavedPalette.pal",4,numberedFileName));
			}
		catch(std::runtime_error)
			{
			/* Ignore errors and carry on: */
			}
		}
	}

VariableManager::VariableManager(const DataSet* sDataSet,const char* sDefaultColorMapName)
	:dataSet(sDataSet),
	 defaultColorMapName(0),
	 scalarVariables(0),
	 colorBarDialogPopup(0),colorBar(0),
	 paletteEditor(0),
	 vectorExtractors(0),
	 currentScalarVariableIndex(-1),currentVectorVariableIndex(-1)
	{
	if(sDefaultColorMapName!=0)
		{
		/* Store the default color map name: */
		int nameLength=strlen(sDefaultColorMapName);
		defaultColorMapName=new char[nameLength+1];
		memcpy(defaultColorMapName,sDefaultColorMapName,nameLength+1);
		}
	
	/* Initialize the scalar variable array: */
	numScalarVariables=dataSet->getNumScalarVariables();
	if(numScalarVariables>0)
		scalarVariables=new ScalarVariable[numScalarVariables];
	
	/* Get the style sheet: */
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	/* Create the color bar dialog: */
	colorBarDialogPopup=new GLMotif::PopupWindow("ColorBarDialogPopup",Vrui::getWidgetManager(),"Color Bar");
	
	/* Create the color bar widget: */
	colorBar=new GLMotif::ColorBar("ColorBar",colorBarDialogPopup,ss.fontHeight*5.0f,6,5);
	
	/* Create the palette editor: */
	paletteEditor=new PaletteEditor;
	paletteEditor->getColorMapChangedCallbacks().add(this,&VariableManager::colorMapChangedCallback);
	paletteEditor->getSavePaletteCallbacks().add(this,&VariableManager::savePaletteCallback);
	
	/* Initialize the vector extractor array: */
	numVectorVariables=dataSet->getNumVectorVariables();
	if(numVectorVariables>0)
		{
		vectorExtractors=new VectorExtractor*[numVectorVariables];
		for(int i=0;i<numVectorVariables;++i)
			vectorExtractors[i]=0;
		}
	
	/* Initialize the current variable state: */
	setCurrentScalarVariable(0);
	setCurrentVectorVariable(0);
	}

VariableManager::~VariableManager(void)
	{
	delete[] defaultColorMapName;
	if(scalarVariables!=0)
		delete[] scalarVariables;
	if(vectorExtractors!=0)
		{
		for(int i=0;i<numVectorVariables;++i)
			delete vectorExtractors[i];
		delete[] vectorExtractors;
		}
	
	delete colorBarDialogPopup;
	delete paletteEditor;
	}

const DataSet* VariableManager::getDataSetByScalarVariable(int scalarVariableIndex) const
	{
	if(scalarVariableIndex<0||scalarVariableIndex>=numScalarVariables)
		return 0;
	
	return dataSet;
	}

const DataSet* VariableManager::getDataSetByVectorVariable(int vectorVariableIndex) const
	{
	if(vectorVariableIndex<0||vectorVariableIndex>=numVectorVariables)
		return 0;
	
	return dataSet;
	}

int VariableManager::getScalarVariable(const char* scalarVariableName) const
	{
	for(int i=0;i<numScalarVariables;++i)
		if(strcmp(getScalarVariableName(i),scalarVariableName)==0)
			return i;
	
	return -1;
	}

int VariableManager::getVectorVariable(const char* vectorVariableName) const
	{
	for(int i=0;i<numVectorVariables;++i)
		if(strcmp(getVectorVariableName(i),vectorVariableName)==0)
			return i;
	
	return -1;
	}

void VariableManager::setCurrentScalarVariable(int newCurrentScalarVariableIndex)
	{
	if(currentScalarVariableIndex==newCurrentScalarVariableIndex||newCurrentScalarVariableIndex<0||newCurrentScalarVariableIndex>=numScalarVariables)
		return;
	
	/* Check if the scalar variable has not been requested before: */
	ScalarVariable& sv=scalarVariables[newCurrentScalarVariableIndex];
	if(sv.scalarExtractor==0)
		prepareScalarVariable(newCurrentScalarVariableIndex);
	
	/* Save the palette editor's current palette: */
	if(currentScalarVariableIndex>=0)
		scalarVariables[currentScalarVariableIndex].palette=paletteEditor->getPalette();
	
	/* Update the current scalar variable: */
	currentScalarVariableIndex=newCurrentScalarVariableIndex;
	
	if(sv.palette==0)
		{
		if(defaultColorMapName!=0)
			{
			/* Load the default palette: */
			try
				{
				paletteEditor->loadPalette(defaultColorMapName,sv.valueRange);
				}
			catch(std::runtime_error)
				{
				/* Create a new palette: */
				paletteEditor->createPalette(GLMotif::ColorMap::GREYSCALE,sv.valueRange);
				}
			}
		else
			{
			/* Create a new palette: */
			paletteEditor->createPalette(GLMotif::ColorMap::GREYSCALE,sv.valueRange);
			}
		}
	else
		{
		/* Upload the previously stored palette: */
		paletteEditor->setPalette(sv.palette);
		delete sv.palette;
		sv.palette=0;
		}
	
	/* Update the palette editor's title: */
	char title[256];
	snprintf(title,sizeof(title),"Palette Editor - %s",dataSet->getScalarVariableName(newCurrentScalarVariableIndex));
	paletteEditor->setTitleString(title);

	/* Update the color bar dialog: */
	snprintf(title,sizeof(title),"Color Bar - %s",dataSet->getScalarVariableName(newCurrentScalarVariableIndex));
	colorBarDialogPopup->setTitleString(title);
	colorBar->setColorMap(sv.colorMap);
	colorBar->setValueRange(sv.valueRange.first,sv.valueRange.second);
	}

void VariableManager::setCurrentVectorVariable(int newCurrentVectorVariableIndex)
	{
	if(currentVectorVariableIndex==newCurrentVectorVariableIndex||newCurrentVectorVariableIndex<0||newCurrentVectorVariableIndex>=numVectorVariables)
		return;
	
	/* Check if the vector variable has not been requested before: */
	if(vectorExtractors[newCurrentVectorVariableIndex]==0)
		vectorExtractors[newCurrentVectorVariableIndex]=dataSet->getVectorExtractor(newCurrentVectorVariableIndex);
	
	/* Update the current vector variable: */
	currentVectorVariableIndex=newCurrentVectorVariableIndex;
	}

const ScalarExtractor* VariableManager::getScalarExtractor(int scalarVariableIndex)
	{
	if(scalarVariableIndex<0||scalarVariableIndex>=numScalarVariables)
		return 0;
	
	/* Check if the scalar variable has not been requested before: */
	if(scalarVariables[scalarVariableIndex].scalarExtractor==0)
		prepareScalarVariable(scalarVariableIndex);
	
	return scalarVariables[scalarVariableIndex].scalarExtractor;
	}

int VariableManager::getScalarVariable(const ScalarExtractor* scalarExtractor) const
	{
	/* Find the scalar extractor among the registered extractors: */
	for(int i=0;i<numScalarVariables;++i)
		if(scalarVariables[i].scalarExtractor==scalarExtractor)
			return i;
	
	return -1;
	}

const DataSet::VScalarRange& VariableManager::getScalarValueRange(int scalarVariableIndex)
	{
	if(scalarVariableIndex<0||scalarVariableIndex>=numScalarVariables)
		return scalarVariables[currentScalarVariableIndex].valueRange;
	
	/* Check if the scalar variable has not been requested before: */
	if(scalarVariables[scalarVariableIndex].scalarExtractor==0)
		prepareScalarVariable(scalarVariableIndex);
	
	return scalarVariables[scalarVariableIndex].valueRange;
	}

const GLColorMap* VariableManager::getColorMap(int scalarVariableIndex)
	{
	if(scalarVariableIndex<0||scalarVariableIndex>=numScalarVariables)
		return 0;
	
	/* Check if the scalar variable has not been requested before: */
	if(scalarVariables[scalarVariableIndex].scalarExtractor==0)
		prepareScalarVariable(scalarVariableIndex);
	
	return scalarVariables[scalarVariableIndex].colorMap;
	}

const VectorExtractor* VariableManager::getVectorExtractor(int vectorVariableIndex)
	{
	if(vectorVariableIndex<0||vectorVariableIndex>=numVectorVariables)
		return 0;
	
	/* Check if the vector variable has not been requested before: */
	if(vectorExtractors[vectorVariableIndex]==0)
		vectorExtractors[vectorVariableIndex]=dataSet->getVectorExtractor(vectorVariableIndex);
	
	return vectorExtractors[vectorVariableIndex];
	}

int VariableManager::getVectorVariable(const VectorExtractor* vectorExtractor) const
	{
	/* Find the vector extractor among the registered extractors: */
	for(int i=0;i<numVectorVariables;++i)
		if(vectorExtractors[i]==vectorExtractor)
			return i;
	
	return -1;
	}

void VariableManager::showColorBar(bool show)
	{
	/* Hide or show color bar dialog based on parameter: */
	if(show)
		Vrui::popupPrimaryWidget(colorBarDialogPopup);
	else
		Vrui::popdownPrimaryWidget(colorBarDialogPopup);
	}

void VariableManager::showPaletteEditor(bool show)
	{
	/* Hide or show color bar dialog based on parameter: */
	if(show)
		Vrui::popupPrimaryWidget(paletteEditor);
	else
		Vrui::popdownPrimaryWidget(paletteEditor);
	}

void VariableManager::createPalette(int newPaletteType)
	{
	/* Define local types: */
	typedef GLMotif::ColorMap ColorMap;
	typedef ColorMap::ValueRange ValueRange;
	typedef ColorMap::ColorMapValue Color;
	typedef ColorMap::ControlPoint ControlPoint;
	
	/* Get the current color map's value range: */
	const ValueRange& valueRange=paletteEditor->getColorMap()->getValueRange();
	double o=valueRange.first;
	double f=valueRange.second-o;
	
	/* Create a new control point vector for the current color map: */
	std::vector<ControlPoint> controlPoints;
	bool createPalette=true;
	switch(newPaletteType)
		{
		case LUMINANCE_GREY:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case LUMINANCE_RED:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(1.0f,0.287f,0.287f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case LUMINANCE_YELLOW:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.564f,0.564f,0.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case LUMINANCE_GREEN:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.0f,0.852f,0.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case LUMINANCE_CYAN:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.0f,0.713f,0.713f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case LUMINANCE_BLUE:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.436f,0.436f,1.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case LUMINANCE_MAGENTA:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(1.0f,0.148f,1.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case SATURATION_RED_CYAN:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.0f,0.287f,0.287f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.0f,0.713f,0.713f,1.0f)));
			break;
		
		case SATURATION_YELLOW_BLUE:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.564f,0.564f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.436f,0.436f,1.0f,1.0f)));
			break;
		
		case SATURATION_GREEN_MAGENTA:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.852f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,0.148f,1.0f,1.0f)));
			break;
		
		case SATURATION_CYAN_RED:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.713f,0.713f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,0.287f,0.287f,1.0f)));
			break;
		
		case SATURATION_BLUE_YELLOW:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.436f,0.436f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.564f,0.564f,0.0f,1.0f)));
			break;
		
		case SATURATION_MAGENTA_GREEN:
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.0f,0.148f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.0f,0.852f,0.0f,1.0f)));
			break;
		
		case RAINBOW:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/5.0),Color(1.0f,0.287f,0.287f,(0.0f/5.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/5.0),Color(0.564f,0.564f,0.0f,(1.0f/5.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/5.0),Color(0.0f,0.852f,0.0f,(2.0f/5.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/5.0),Color(0.0f,0.713f,0.713f,(3.0f/5.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/5.0),Color(0.436f,0.436f,1.0f,(4.0f/5.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/5.0),Color(1.0f,0.148f,1.0f,(5.0f/5.0f))));
			break;
			}

		case GOCAD_AFRICA:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/3.0),Color(0.804f,0.694f,0.451f,(0.0f/3.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/3.0),Color(0.852f,0.852f,0.000f,(1.0f/3.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/3.0),Color(0.000f,0.852f,0.000f,(2.0f/3.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/3.0),Color(1.000f,0.287f,0.287f,(3.0f/3.0f))));
			break;
			}

		case GOCAD_BGR:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.436f,0.436f,1.000f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.000f,0.8520f,0.000f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.000f,0.287f,0.0287f,1.0f)));
			break;
			}

		case GOCAD_CLASSIC:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/4.0),Color(0.436f,0.436f,1.000f,(0.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/4.0),Color(0.000f,0.713f,0.713f,(1.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/4.0),Color(0.000f,0.852f,0.000f,(2.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/4.0),Color(0.564f,0.564f,0.000f,(3.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/4.0),Color(1.000f,0.287f,0.287f,(4.0f/4.0f))));
			break;
			}

		case GOCAD_FLAG:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.463f,0.463f,1.000f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(1.000f,1.000f,1.000f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.000f,0.287f,0.287f,1.0f)));
			break;
			}

		case GOCAD_FLUIDS:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.612f,0.906f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.514f,0.776f,0.286f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.929f,0.220f,0.290f,1.0f)));
			break;
			}
		
		case GOCAD_GREEN_YELLOW:
         {
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.000f,0.852f,0.000f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.952f,0.952f,0.000f,1.0f)));
			break;
         }

		case GOCAD_RAINBOW_0:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.000f,0.000f,0.000f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.000f,0.298f,1.000f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.000f,0.852f,0.000f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.952f,0.952f,0.000f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(1.000f,0.682f,0.200f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(1.000f,0.287f,0.287f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(1.000f,1.000f,1.000f,(6.0f/6.0f))));
			break;
			}

		case GOCAD_RAINBOW_1:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(1.000f,0.287f,0.287f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.952f,0.952f,0.000f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.000f,0.852f,0.000f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.000f,0.713f,0.713f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.436f,0.436f,1.000f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(1.000f,0.148f,1.000f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(1.000f,1.000f,1.000f,(6.0f/6.0f))));
			break;
			}

		case GOCAD_RAINBOW_2:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/10.0),Color(0.004f,1.000f,0.004f,(0.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/10.0),Color(0.612f,0.008f,0.714f,(1.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/10.0),Color(1.000f,1.000f,0.008f,(2.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/10.0),Color(0.671f,0.008f,0.008f,(3.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/10.0),Color(0.635f,1.000f,0.624f,(4.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/10.0),Color(0.063f,0.031f,1.000f,(5.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/10.0),Color(1.000f,0.627f,0.820f,(6.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(7.0/10.0),Color(0.047f,1.000f,0.969f,(7.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(8.0/10.0),Color(0.012f,0.525f,0.012f,(8.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(9.0/10.0),Color(1.000f,0.287f,0.287f,(9.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(10.0/10.0),Color(0.000f,0.000f,0.000f,(10.0f/10.0f))));
			break;
			}

		case GOCAD_RED_BLUE:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.000f,0.287f,0.287f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.436f,0.436f,1.000f,1.0f)));
			break;
			}

		case GOCAD_WHITE_BLUE:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.000f,1.000f,1.000f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.436f,0.436f,1.000f,1.0f)));
			break;
			}

		case QUALITATIVE_0:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.498f,0.788f,0.502f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.745f,0.682f,0.831f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.992f,0.753f,0.525f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(1.000f,1.000f,0.600f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.220f,0.424f,0.690f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.941f,0.148f,1.000f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.749f,0.357f,0.090f,(6.0f/6.0f))));
			break;
			}

		case QUALITATIVE_1:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.106f,0.620f,0.467f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.851f,0.373f,0.008f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.459f,0.439f,0.702f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.906f,0.161f,0.541f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.004f,0.651f,0.118f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.902f,0.671f,0.008f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.651f,0.463f,0.114f,(6.0f/6.0f))));
			break;
			}

		case QUALITATIVE_2:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.651f,0.808f,0.890f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.122f,0.471f,0.706f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.698f,0.875f,0.541f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.200f,0.627f,0.173f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.984f,0.604f,0.600f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.890f,0.102f,0.110f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.992f,0.749f,0.435f,(6.0f/6.0f))));
			break;
			}

		case QUALITATIVE_3:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.984f,0.706f,0.682f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.702f,0.801f,0.890f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.800f,0.922f,0.773f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.871f,0.796f,0.894f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.996f,0.851f,0.651f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(1.000f,1.000f,0.800f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.898f,0.847f,0.741f,(6.0f/6.0f))));
			break;
			}

		case QUALITATIVE_4:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.702f,0.886f,0.804f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.992f,0.804f,0.675f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.796f,0.835f,0.910f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.957f,0.792f,0.894f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.902f,0.961f,0.788f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(1.000f,0.949f,0.682f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.945f,0.886f,0.804f,(6.0f/6.0f))));
			break;
			}

		case QUALITATIVE_5:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.894f,0.102f,0.110f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.216f,0.494f,0.722f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.302f,0.686f,0.290f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.596f,0.306f,0.639f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(1.000f,0.498f,0.000f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.952f,0.952f,0.200f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.651f,0.337f,0.157f,(6.0f/6.0f))));
			break;
			}

		case QUALITATIVE_6:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.400f,0.761f,0.647f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.988f,0.553f,0.384f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.553f,0.627f,0.796f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.906f,0.541f,0.764f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.651f,0.847f,0.329f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(1.000f,0.851f,0.184f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.898f,0.769f,0.580f,(6.0f/6.0f))));
			break;
			}

		case QUALITATIVE_7:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.553f,0.827f,0.780f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(1.000f,1.000f,0.702f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.745f,0.729f,0.855f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.984f,0.502f,0.447f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.502f,0.694f,0.827f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.992f,0.706f,0.384f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.702f,0.871f,0.413f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_0:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.549f,0.318f,0.039f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.847f,0.702f,0.396f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.964f,0.910f,0.765f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.961f,0.961f,0.961f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.780f,0.918f,0.937f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.353f,0.706f,0.675f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.004f,0.400f,0.369f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_1:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.773f,0.106f,0.491f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.914f,0.639f,0.788f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.992f,0.918f,0.937f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.969f,0.969f,0.969f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.902f,0.961f,0.816f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.631f,0.843f,0.416f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.302f,0.573f,0.129f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_2:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.463f,0.165f,0.514f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.686f,0.553f,0.765f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.906f,0.831f,0.910f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.969f,0.969f,0.969f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.851f,0.941f,0.827f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.498f,0.749f,0.482f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.106f,0.471f,0.216f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_3:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.902f,0.380f,0.004f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.945f,0.639f,0.251f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.996f,0.878f,0.714f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.969f,0.969f,0.969f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.847f,0.855f,0.922f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.600f,0.557f,0.765f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.329f,0.153f,0.533f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_4:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.698f,0.094f,0.169f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.937f,0.541f,0.384f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.992f,0.859f,0.780f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.969f,0.969f,0.969f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.820f,0.898f,0.941f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.404f,0.663f,0.812f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.129f,0.400f,0.675f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_5:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.698f,0.094f,0.169f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.937f,0.541f,0.384f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.992f,0.859f,0.780f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(1.000f,1.000f,1.000f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.878f,0.878f,0.878f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.600f,0.600f,0.600f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.302f,0.302f,0.302f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_6:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.843f,0.188f,0.153f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.988f,0.553f,0.349f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.996f,0.878f,0.565f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(1.000f,1.000f,0.749f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.878f,0.953f,0.973f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.569f,0.749f,0.859f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.271f,0.459f,0.706f,(6.0f/6.0f))));
			break;
			}

		case DIVERGING_7:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.843f,0.188f,0.153f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.988f,0.553f,0.349f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.996f,0.878f,0.545f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(1.000f,1.000f,0.749f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.851f,0.937f,0.545f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(0.569f,0.812f,0.376f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(0.102f,0.596f,0.314f,(6.0f/6.0f))));
			break;
			}

		default:
			createPalette=false;
		}
	
	if(createPalette)
		{
		/* Create the new color map: */
		paletteEditor->createPalette(controlPoints);
		
		Vrui::requestUpdate();
		}
	}

void VariableManager::loadPalette(const char* paletteFileName)
	{
	/* Load the given palette file: */
	paletteEditor->loadPalette(paletteFileName,scalarVariables[currentScalarVariableIndex].valueRange);
	}

void VariableManager::insertPaletteEditorControlPoint(double newControlPoint)
	{
	paletteEditor->getColorMap()->insertControlPoint(newControlPoint);
	}

}

}
