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
			controlPoints.push_back(ControlPoint(o+f*(1.0/3.0),Color(1.0f,1.0f,0.0f,(1.0f/3.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/3.0),Color(0.0f,1.0f,0.0f,(2.0f/3.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/3.0),Color(1.0f,0.0f,0.0f,(3.0f/3.0f))));
			break;
			}

		case GOCAD_BGR:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.0f,1.0f,0.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,0.0f,0.0f,1.0f)));
			break;
			}

		case GOCAD_CLASSIC:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/4.0),Color(0.0f,0.0f,1.0f,(0.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/4.0),Color(0.0f,1.0f,1.0f,(1.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/4.0),Color(0.0f,1.0f,0.0f,(2.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/4.0),Color(1.0f,1.0f,0.0f,(3.0f/4.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/4.0),Color(1.0f,0.0f,0.0f,(4.0f/4.0f))));
			break;
			}

		case GOCAD_FLAG:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(1.0f,1.0f,1.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,0.0f,0.0f,1.0f)));
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
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,1.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,0.0f,1.0f)));
			break;
         }

		case GOCAD_RAINBOW_0:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(0.0f,0.0f,0.0f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(0.0f,0.298f,1.0f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.0f,1.0f,0.282f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(1.0f,1.0f,0.0f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(1.0f,0.482f,0.0f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(1.0f,0.0f,0.0f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(1.0f,1.0f,1.0f,(6.0f/6.0f))));
			break;
			}

		case GOCAD_RAINBOW_1:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/6.0),Color(1.0f,0.0f,0.0f,(0.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/6.0),Color(1.0f,1.0f,0.0f,(1.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/6.0),Color(0.0f,1.0f,0.0f,(2.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/6.0),Color(0.0f,1.0f,1.0f,(3.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/6.0),Color(0.0f,0.0f,1.0f,(4.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/6.0),Color(1.0f,0.0f,1.0f,(5.0f/6.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/6.0),Color(1.0f,1.0f,1.0f,(6.0f/6.0f))));
			break;
			}

		case GOCAD_RAINBOW_2:
			{
			double o=valueRange.first;
			double f=valueRange.second-o;
			controlPoints.push_back(ControlPoint(o+f*(0.0/10.0),Color(0.004f,1.0f,0.004f,(0.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(1.0/10.0),Color(0.612f,0.008f,0.714f,(1.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(2.0/10.0),Color(1.0f,1.0f,0.008f,(2.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(3.0/10.0),Color(0.671f,0.008f,0.008f,(3.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(4.0/10.0),Color(0.635f,1.0f,0.624f,(4.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(5.0/10.0),Color(0.063f,0.031f,1.0f,(5.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(6.0/10.0),Color(1.0f,0.627f,0.820f,(6.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(7.0/10.0),Color(0.047f,1.0f,0.969f,(7.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(8.0/10.0),Color(0.012f,0.525f,0.012f,(8.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(9.0/10.0),Color(1.0f,0.008f,0.008f,(9.0f/10.0f))));
			controlPoints.push_back(ControlPoint(o+f*(10.0/10.0),Color(0.0f,0.0f,0.0f,(10.0f/10.0f))));
			break;
			}

		case GOCAD_RED_BLUE:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.0f,0.0f,1.0f,1.0f)));
			break;
			}

		case GOCAD_WHITE_BLUE:
			{
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.0f,1.0f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.0f,0.0f,1.0f,1.0f)));
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
