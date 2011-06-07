/***********************************************************************
Visualizer - Test application for the new visualization component
framework.
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
#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#include <Misc/ThrowStdErr.h>
#include <Misc/Timer.h>
#include <Misc/File.h>
#include <Misc/CreateNumberedFileName.h>
#include <Geometry/OrthogonalTransformation.h>
#include <GL/gl.h>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <GL/GLGeometryWrappers.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Menu.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/WidgetManager.h>
#include <Vrui/Vrui.h>

#include <Abstract/DataSetRenderer.h>
#include <Abstract/Module.h>
#include <Abstract/Algorithm.h>
#include <Abstract/Element.h>

#include "ColorBar.h"

#include "Visualizer.h"

namespace {

/****************
Helper functions:
****************/

std::string readToken(Misc::File& file,int& nextChar)
	{
	/* Skip whitespace: */
	while(nextChar!=EOF&&isspace(nextChar))
		nextChar=file.getc();
	
	/* Read the next token: */
	std::string result="";
	if(nextChar=='"')
		{
		/* Skip the opening quote: */
		nextChar=file.getc();
		
		/* Read a quoted token: */
		while(nextChar!=EOF&&nextChar!='"')
			{
			result+=char(nextChar);
			nextChar=file.getc();
			}
		
		if(nextChar=='"')
			nextChar=file.getc();
		else
			Misc::throwStdErr("unterminated quoted token in input file");
		}
	else
		{
		/* Read an unquoted token: */
		while(nextChar!=EOF&&!isspace(nextChar))
			{
			result+=char(nextChar);
			nextChar=file.getc();
			}
		}
	
	return result;
	}

}

/***************************
Methods of class Visualizer:
***************************/

GLMotif::Popup* Visualizer::createRenderingModesMenu(void)
	{
	GLMotif::Popup* renderingModesMenuPopup=new GLMotif::Popup("RenderingModesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* renderingModes=new GLMotif::RadioBox("RenderingModes",renderingModesMenuPopup,false);
	renderingModes->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	int numRenderingModes=dataSetRenderer->getNumRenderingModes();
	for(int i=0;i<numRenderingModes;++i)
		renderingModes->addToggle(dataSetRenderer->getRenderingModeName(i));
	
	renderingModes->setSelectedToggle(dataSetRenderer->getRenderingMode());
	renderingModes->getValueChangedCallbacks().add(this,&Visualizer::changeRenderingModeCallback);
	
	renderingModes->manageChild();
	
	return renderingModesMenuPopup;
	}

GLMotif::Popup* Visualizer::createScalarVariablesMenu(void)
	{
	GLMotif::Popup* scalarVariablesMenuPopup=new GLMotif::Popup("ScalarVariablesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* scalarVariables=new GLMotif::RadioBox("ScalarVariables",scalarVariablesMenuPopup,false);
	scalarVariables->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	int numScalarVariables=dataSet->getNumScalarVariables();
	for(int i=0;i<numScalarVariables;++i)
		scalarVariables->addToggle(dataSet->getScalarVariableName(i));
	
	scalarVariables->setSelectedToggle(scalarVariable);
	scalarVariables->getValueChangedCallbacks().add(this,&Visualizer::changeScalarVariableCallback);
	
	scalarVariables->manageChild();
	
	return scalarVariablesMenuPopup;
	}

GLMotif::Popup* Visualizer::createVectorVariablesMenu(void)
	{
	GLMotif::Popup* vectorVariablesMenuPopup=new GLMotif::Popup("VectorVariablesMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* vectorVariables=new GLMotif::RadioBox("VectorVariables",vectorVariablesMenuPopup,false);
	vectorVariables->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	int numVectorVariables=dataSet->getNumVectorVariables();
	for(int i=0;i<numVectorVariables;++i)
		vectorVariables->addToggle(dataSet->getVectorVariableName(i));
	
	vectorVariables->setSelectedToggle(vectorVariable);
	vectorVariables->getValueChangedCallbacks().add(this,&Visualizer::changeVectorVariableCallback);
	
	vectorVariables->manageChild();
	
	return vectorVariablesMenuPopup;
	}

GLMotif::Popup* Visualizer::createAlgorithmsMenu(void)
	{
	GLMotif::Popup* algorithmsMenuPopup=new GLMotif::Popup("AlgorithmsMenuPopup",Vrui::getWidgetManager());
	
	GLMotif::RadioBox* algorithms=new GLMotif::RadioBox("Algorithms",algorithmsMenuPopup,false);
	algorithms->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
	
	/* Add the cutting plane algorithm: */
	int algorithmIndex=0;
	algorithms->addToggle("Cutting Plane");
	++algorithmIndex;
	
	if(dataSet->getNumScalarVariables()>0)
		{
		/* Add the scalar evaluator algorithm: */
		algorithms->addToggle("Evaluate Scalars");
		++algorithmIndex;
		
		/* Add scalar algorithms: */
		firstScalarAlgorithmIndex=algorithmIndex;
		for(int i=0;i<module->getNumScalarAlgorithms();++i)
			{
			algorithms->addToggle(module->getScalarAlgorithmName(i));
			++algorithmIndex;
			}
		}
	
	if(dataSet->getNumVectorVariables()>0)
		{
		/* Add the vector evaluator algorithm: */
		algorithms->addToggle("Evaluate Vectors");
		++algorithmIndex;
		
		/* Add vector algorithms: */
		firstVectorAlgorithmIndex=algorithmIndex;
		for(int i=0;i<module->getNumVectorAlgorithms();++i)
			{
			algorithms->addToggle(module->getVectorAlgorithmName(i));
			++algorithmIndex;
			}
		}
		
	algorithms->setSelectedToggle(algorithm);
	algorithms->getValueChangedCallbacks().add(this,&Visualizer::changeAlgorithmCallback);
	
	algorithms->manageChild();
	
	return algorithmsMenuPopup;
	}

GLMotif::Popup* Visualizer::createStandardLuminancePalettesMenu(void)
	{
	GLMotif::Popup* standardLuminancePalettesMenuPopup=new GLMotif::Popup("StandardLuminancePalettesMenuPopup",Vrui::getWidgetManager());
	
	/* Create the palette creation menu and add entries for all standard palettes: */
	GLMotif::SubMenu* standardLuminancePalettes=new GLMotif::SubMenu("StandardLuminancePalettes",standardLuminancePalettesMenuPopup,false);
	
	standardLuminancePalettes->addEntry("Grey");
	standardLuminancePalettes->addEntry("Red");
	standardLuminancePalettes->addEntry("Yellow");
	standardLuminancePalettes->addEntry("Green");
	standardLuminancePalettes->addEntry("Cyan");
	standardLuminancePalettes->addEntry("Blue");
	standardLuminancePalettes->addEntry("Magenta");
	
	standardLuminancePalettes->getEntrySelectCallbacks().add(this,&Visualizer::createStandardLuminancePaletteCallback);
	
	standardLuminancePalettes->manageChild();
	
	return standardLuminancePalettesMenuPopup;
	}

GLMotif::Popup* Visualizer::createStandardSaturationPalettesMenu(void)
	{
	GLMotif::Popup* standardSaturationPalettesMenuPopup=new GLMotif::Popup("StandardSaturationPalettesMenuPopup",Vrui::getWidgetManager());
	
	/* Create the palette creation menu and add entries for all standard palettes: */
	GLMotif::SubMenu* standardSaturationPalettes=new GLMotif::SubMenu("StandardSaturationPalettes",standardSaturationPalettesMenuPopup,false);
	
	standardSaturationPalettes->addEntry("Red -> Cyan");
	standardSaturationPalettes->addEntry("Yellow -> Blue");
	standardSaturationPalettes->addEntry("Green -> Magenta");
	standardSaturationPalettes->addEntry("Cyan -> Red");
	standardSaturationPalettes->addEntry("Blue -> Yellow");
	standardSaturationPalettes->addEntry("Magenta -> Green");
	standardSaturationPalettes->addEntry("Rainbow");
	
	standardSaturationPalettes->getEntrySelectCallbacks().add(this,&Visualizer::createStandardSaturationPaletteCallback);
	
	standardSaturationPalettes->manageChild();
	
	return standardSaturationPalettesMenuPopup;
	}

GLMotif::Popup* Visualizer::createColorMenu(void)
	{
	GLMotif::Popup* colorMenuPopup=new GLMotif::Popup("ColorMenuPopup",Vrui::getWidgetManager());
	
	/* Create the color menu and add entries for all standard palettes: */
	GLMotif::SubMenu* colorMenu=new GLMotif::SubMenu("ColorMenu",colorMenuPopup,false);
	
	GLMotif::CascadeButton* standardLuminancePalettesCascade=new GLMotif::CascadeButton("StandardLuminancePalettesCascade",colorMenu,"Create LuminancePalette");
	standardLuminancePalettesCascade->setPopup(createStandardLuminancePalettesMenu());
	
	GLMotif::CascadeButton* standardSaturationPalettesCascade=new GLMotif::CascadeButton("StandardSaturationPalettesCascade",colorMenu,"Create SaturationPalette");
	standardSaturationPalettesCascade->setPopup(createStandardSaturationPalettesMenu());
	
	GLMotif::ToggleButton* showColorBarToggle=new GLMotif::ToggleButton("ShowColorBarToggle",colorMenu,"Show Color Bar");
	showColorBarToggle->getValueChangedCallbacks().add(this,&Visualizer::showColorBarCallback);
	
	GLMotif::ToggleButton* showPaletteEditorToggle=new GLMotif::ToggleButton("ShowPaletteEditorToggle",colorMenu,"Show Palette Editor");
	showPaletteEditorToggle->getValueChangedCallbacks().add(this,&Visualizer::showPaletteEditorCallback);
	
	colorMenu->manageChild();
	
	return colorMenuPopup;
	}

GLMotif::PopupMenu* Visualizer::createMainMenu(void)
	{
	GLMotif::PopupMenu* mainMenuPopup=new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
	mainMenuPopup->setTitle("3D Visualizer");
	
	GLMotif::Menu* mainMenu=new GLMotif::Menu("MainMenu",mainMenuPopup,false);
	
	GLMotif::CascadeButton* renderingModesCascade=new GLMotif::CascadeButton("RenderingModesCascade",mainMenu,"Rendering Modes");
	renderingModesCascade->setPopup(createRenderingModesMenu());
	
	if(dataSet->getNumScalarVariables()>0)
		{
		GLMotif::CascadeButton* scalarVariablesCascade=new GLMotif::CascadeButton("ScalarVariablesCascade",mainMenu,"Scalar Variables");
		scalarVariablesCascade->setPopup(createScalarVariablesMenu());
		}
	
	if(dataSet->getNumVectorVariables()>0)
		{
		GLMotif::CascadeButton* vectorVariablesCascade=new GLMotif::CascadeButton("VectorVariablesCascade",mainMenu,"Vector Variables");
		vectorVariablesCascade->setPopup(createVectorVariablesMenu());
		}
	
	GLMotif::CascadeButton* algorithmsCascade=new GLMotif::CascadeButton("AlgorithmsCascade",mainMenu,"Algorithms");
	algorithmsCascade->setPopup(createAlgorithmsMenu());
	
	showElementListToggle=new GLMotif::ToggleButton("ShowElementListToggle",mainMenu,"Show Element List");
	showElementListToggle->getValueChangedCallbacks().add(this,&Visualizer::showElementListCallback);
	
	GLMotif::Button* clearElementsButton=new GLMotif::Button("ClearElementsButton",mainMenu,"Clear Visualization Elements");
	clearElementsButton->getSelectCallbacks().add(this,&Visualizer::clearElementsCallback);
	
	GLMotif::CascadeButton* colorCascade=new GLMotif::CascadeButton("ColorCascade",mainMenu,"Color Maps");
	colorCascade->setPopup(createColorMenu());
	
	GLMotif::Button* centerDisplayButton=new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
	centerDisplayButton->getSelectCallbacks().add(this,&Visualizer::centerDisplayCallback);
	
	mainMenu->manageChild();
	
	return mainMenuPopup;
	}

GLMotif::PopupWindow* Visualizer::createElementListDialog(void)
	{
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* elementListDialogPopup=new GLMotif::PopupWindow("ElementListDialogPopup",Vrui::getWidgetManager(),"Visualization Element List");
	
	elementListDialog=new GLMotif::RowColumn("ElementListDialog",elementListDialogPopup,false);
	elementListDialog->setNumMinorWidgets(3);
	
	elementListDialog->manageChild();
	
	return elementListDialogPopup;
	}


GLMotif::PopupWindow* Visualizer::createColorBarDialog(void)
	{
	/* Create a color bar dialog: */
	GLMotif::PopupWindow* colorBarDialogPopup=new GLMotif::PopupWindow("ColorBarDialogPopup",Vrui::getWidgetManager(),"Color Bar");
	
	/* Get the style sheet: */
	const GLMotif::StyleSheet& ss=*Vrui::getWidgetManager()->getStyleSheet();
	
	/* Create the color bar widget: */
	colorBar=new GLMotif::ColorBar("ColorBar",colorBarDialogPopup,ss.fontHeight*5.0f,6,5);
	
	return colorBarDialogPopup;
	}

void Visualizer::addElement(Visualizer::Element* newElement)
	{
	/* Create the element's list structure: */
	ListElement le;
	le.element=newElement;
	le.name=newElement->getName();
	le.settingsDialog=newElement->createSettingsDialog(Vrui::getWidgetManager());
	le.settingsDialogVisible=false;
	le.show=true;
	
	/* Add the element to the list: */
	elements.push_back(le);
	
	/* Create an entry in the element list dialog: */
	GLMotif::TextField* elementName=new GLMotif::TextField("ElementName",elementListDialog,20);
	elementName->setLabel(le.name.c_str());
	
	GLMotif::ToggleButton* showSettingsDialogToggle=new GLMotif::ToggleButton("ShowSettingsDialogToggle",elementListDialog,"Show Dialog");
	showSettingsDialogToggle->getValueChangedCallbacks().add(this,&Visualizer::showElementSettingsDialogCallback);
	
	GLMotif::ToggleButton* showElementToggle=new GLMotif::ToggleButton("ShowElementToggle",elementListDialog,"Show");
	showElementToggle->setToggle(le.show);
	showElementToggle->getValueChangedCallbacks().add(this,&Visualizer::showElementCallback);
	}

void Visualizer::selectScalarVariable(int newScalarVariable)
	{
	if(newScalarVariable>=0&&newScalarVariable<dataSet->getNumScalarVariables()&&newScalarVariable!=scalarVariable)
		{
		/* Save the current palette stored in the palette editor: */
		if(scalarVariable>=0)
			palettes[scalarVariable]=paletteEditor->getPalette();
		
		/* Set the new scalar variable: */
		scalarVariable=newScalarVariable;
		
		/* Get a new scalar extractor: */
		delete scalarExtractor;
		scalarExtractor=dataSet->getScalarExtractor(scalarVariable);
		
		/* Create a new palette if the scalar variable is selected for the first time: */
		DataSet::VScalarRange scalarRange;
		if(palettes[scalarVariable]==0)
			{
			/* Create a dummy 256-entry OpenGL color map for rendering: */
			colorMaps[scalarVariable]=new GLColorMap(GLColorMap::GREYSCALE|GLColorMap::CONSTANT_ALPHA,1.0f,1.0f,0.0,1.0);
			
			/* Calculate the scalar variable's value range: */
			scalarRange=dataSet->calcScalarValueRange(scalarExtractor);
			
			/* Upload a new palette into the palette editor: */
			if(colorMapName!=0)
				{
				/* Load the palette from the given file: */
				try
					{
					paletteEditor->loadPalette(colorMapName,scalarRange);
					}
				catch(std::runtime_error)
					{
					/* Create a default palette instead: */
					paletteEditor->createPalette(GLMotif::ColorMap::GREYSCALE,scalarRange);
					}
				}
			else
				{
				/* Create a default palette: */
				paletteEditor->createPalette(GLMotif::ColorMap::GREYSCALE,scalarRange);
				}
			}
		else
			{
			/* Upload the stored palette to the palette editor: */
			paletteEditor->setPalette(palettes[scalarVariable]);
			delete palettes[scalarVariable];
			palettes[scalarVariable]=0;
			scalarRange=DataSet::VScalarRange(paletteEditor->getColorMap()->getValueRange());
			}
		
		/* Update the palette editor's title: */
		char title[256];
		snprintf(title,sizeof(title),"Palette Editor - %s",dataSet->getScalarVariableName(scalarVariable));
		paletteEditor->setTitleString(title);
		
		/* Update the color bar dialog: */
		snprintf(title,sizeof(title),"Color Bar - %s",dataSet->getScalarVariableName(scalarVariable));
		colorBarDialogPopup->setTitleString(title);
		colorBar->setColorMap(colorMaps[scalarVariable]);
		colorBar->setValueRange(scalarRange.first,scalarRange.second);
		}
	}

Visualizer::Visualizer(int& argc,char**& argv,char**& appDefaults)
	:Vrui::Application(argc,argv,appDefaults),
	 moduleManager(VISUALIZER_MODULENAMETEMPLATE),
	 module(0),
	 dataSet(0),
	 colorMapName(0),colorMaps(0),
	 scalarExtractor(0),
	 vectorExtractor(0),
	 dataSetRenderer(0),
	 firstScalarAlgorithmIndex(0),firstVectorAlgorithmIndex(0),
	 numCuttingPlanes(0),cuttingPlanes(0),
	 scalarVariable(-1),vectorVariable(0),algorithm(0),
	 mainMenu(0),
	 showElementListToggle(0),
	 elementListDialogPopup(0),elementListDialog(0),
	 colorBarDialogPopup(0),colorBar(0),
	 paletteEditor(0),palettes(0)
	{
	/* Parse the command line: */
	std::string moduleClassName="";
	std::vector<std::string> dataSetArgs;
	const char* argColorMapName=0;
	const char* viewFileName=0;
	for(int i=1;i<argc;++i)
		{
		if(argv[i][0]=='-')
			{
			if(strcasecmp(argv[i]+1,"PALETTE")==0)
				{
				++i;
				argColorMapName=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"VIEW")==0)
				{
				++i;
				viewFileName=argv[i];
				}
			else if(strcasecmp(argv[i]+1,"CLASS")==0)
				{
				/* Get visualization module class name and data set arguments from command line: */
				++i;
				moduleClassName=argv[i];
				++i;
				while(i<argc)
					{
					dataSetArgs.push_back(argv[i]);
					++i;
					}
				}
			}
		else
			{
			/* Read the meta-input file of the given name: */
			Misc::File inputFile(argv[i],"rt");
			
			/* Parse the meta-input file: */
			int nextChar=inputFile.getc();
			
			/* Read the module class name: */
			moduleClassName=readToken(inputFile,nextChar);
			
			/* Read the data set arguments: */
			dataSetArgs.clear();
			while(true)
				{
				/* Read the next argument: */
				std::string arg=readToken(inputFile,nextChar);
				if(arg=="")
					break;

				/* Store the argument in the list: */
				dataSetArgs.push_back(arg);
				}
			}
		}
	
	/* Check if a module class name and data set arguments were provided: */
	if(moduleClassName=="")
		Misc::throwStdErr("Visualizer::Visualizer: no visualization module class name provided");
	if(dataSetArgs.empty())
		Misc::throwStdErr("Visualizer::Visualizer: no data set arguments provided");
	
	/* Load a visualization module and a data set: */
	try
		{
		/* Load the appropriate visualization module: */
		module=moduleManager.loadClass(moduleClassName.c_str());
		
		/* Load a data set: */
		Misc::Timer t;
		dataSet=module->load(dataSetArgs);
		t.elapse();
		if(Vrui::isMaster())
			std::cout<<"Time to load data set: "<<t.getTime()*1000.0<<" ms"<<std::endl;
		}
	catch(std::runtime_error err)
		{
		Misc::throwStdErr("Visualizer::Visualizer: Could not load data set due to exception %s",err.what());
		}
	
	/* Determine the color to render the data set: */
	for(int i=0;i<3;++i)
		dataSetRenderColor[i]=1.0f-Vrui::getBackgroundColor()[i];
	dataSetRenderColor[3]=0.2f;
	
	/* Save the color map name for later: */
	if(argColorMapName!=0)
		{
		colorMapName=new char[strlen(argColorMapName)+1];
		strcpy(colorMapName,argColorMapName);
		}
	
	/* Create an empty color map for each scalar value in the data set: */
	colorMaps=new GLColorMap*[dataSet->getNumScalarVariables()];
	for(int i=0;i<dataSet->getNumScalarVariables();++i)
		colorMaps[i]=0;
	
	/* Create the color bar dialog: */
	colorBarDialogPopup=createColorBarDialog();
	
	/* Create a palette editor: */
	paletteEditor=new PaletteEditor;
	paletteEditor->getColorMapChangedCallbacks().add(this,&Visualizer::colorMapChangedCallback);
	paletteEditor->getSavePaletteCallbacks().add(this,&Visualizer::savePaletteCallback);
	palettes=new PaletteEditor::Storage*[dataSet->getNumScalarVariables()];
	for(int i=0;i<dataSet->getNumScalarVariables();++i)
		palettes[i]=0;
	
	/* Select the initial scalar variable: */
	selectScalarVariable(0);
	
	/* Select the initial vector variable: */
	if(vectorVariable<dataSet->getNumVectorVariables())
		vectorExtractor=dataSet->getVectorExtractor(vectorVariable);
	
	/* Create a data set renderer: */
	dataSetRenderer=module->getRenderer(dataSet);
	
	/* Create cutting planes: */
	numCuttingPlanes=6;
	cuttingPlanes=new CuttingPlane[numCuttingPlanes];
	for(size_t i=0;i<numCuttingPlanes;++i)
		{
		cuttingPlanes[i].allocated=false;
		cuttingPlanes[i].active=false;
		}
	
	/* Create the main menu: */
	mainMenu=createMainMenu();
	Vrui::setMainMenu(mainMenu);
	
	/* Create the element list dialog: */
	elementListDialogPopup=createElementListDialog();
	
	/* Initialize navigation transformation: */
	if(viewFileName!=0)
		{
		/* Open viewpoint file: */
		Misc::File viewpointFile(viewFileName,"rb",Misc::File::LittleEndian);
		
		/* Read the navigation transformation: */
		Vrui::NavTransform::Vector translation;
		viewpointFile.read(translation.getComponents(),3);
		Vrui::NavTransform::Rotation::Scalar quaternion[4];
		viewpointFile.read(quaternion,4);
		Vrui::NavTransform::Scalar scaling=viewpointFile.read<Vrui::NavTransform::Scalar>();
		
		/* Set the navigation transformation: */
		Vrui::setNavigationTransformation(Vrui::NavTransform(translation,Vrui::NavTransform::Rotation::fromQuaternion(quaternion),scaling));
		}
	else
		centerDisplayCallback(0);
	
	#if 0
	/* Create a slice: */
	DataSet::Locator* locator=dataSet->getLocator();
	DataSet::Point p2(33.2,31.8,31.8); // Central slice in large geode dataset
	locator->setPosition(p2);
	locator->setOrientation(DataSet::Orientation::identity);
	Algorithm* algorithm=module->getScalarAlgorithm(0,colorMaps[0],dataSet,scalarExtractor);
	Misc::Timer t;
	Element* element=algorithm->createElement(locator);
	t.elapse();
	std::cout<<"Time to create visualization element: "<<t.getTime()*1000.0<<" ms"<<std::endl;
	addElement(element);
	delete algorithm;
	delete locator;
	#endif
	
	#if 0
	/* Create an isosurface: */
	DataSet::Locator* locator=dataSet->getLocator();
	// DataSet::Point p2(30.1957,31.8198,32.6442); // Huge isosurface in large geode dataset
	// DataSet::Point p2(774.6164,25000.2413,27550.9450);
	// DataSet::Point p2(118.385,179.2,139.872); // Huge isosurface (25.8659) in teddybear
	// DataSet::Point p2(20.0,20.0,20.0); // Central point in Marschner-Lobb dataset
	DataSet::Point p2(-2604.9651,-1363.0122,5521.5595); // Feature 1 in user study data set
	locator->setPosition(p2);
	Algorithm* algorithm=module->getScalarAlgorithm(2,colorMaps[0],dataSet,scalarExtractor);
	Misc::Timer t;
	Element* element=algorithm->createElement(locator);
	t.elapse();
	std::cout<<"Time to create visualization element: "<<t.getTime()*1000.0<<" ms"<<std::endl;
	addElement(element);
	delete algorithm;
	delete locator;
	#endif
	}

Visualizer::~Visualizer(void)
	{
	delete mainMenu;
	delete elementListDialogPopup;
	
	/* Delete all finished visualization elements: */
	for(ElementList::iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		{
		if(veIt->settingsDialogVisible)
			Vrui::popdownPrimaryWidget(veIt->settingsDialog);
		delete veIt->settingsDialog;
		}
	
	/* Delete all locators: */
	for(BaseLocatorList::iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		delete *blIt;
	
	/* Delete the cutting planes: */
	delete[] cuttingPlanes;
	
	/* Delete the data set renderer: */
	delete dataSetRenderer;
	
	/* Delete the scalar and vector extractors: */
	delete scalarExtractor;
	delete vectorExtractor;
	
	/* Delete the palette editor: */
	delete paletteEditor;
	if(palettes!=0)
		{
		for(int i=0;i<dataSet->getNumScalarVariables();++i)
			delete palettes[i];
		delete[] palettes;
		}
	
	/* Delete the color bar: */
	delete colorBarDialogPopup;
	
	/* Delete all color maps: */
	if(colorMaps!=0)
		{
		for(int i=0;i<dataSet->getNumScalarVariables();++i)
			delete colorMaps[i];
		delete[] colorMaps;
		}
	delete[] colorMapName;
	
	/* Delete the data set: */
	delete dataSet;
	}

void Visualizer::toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData)
	{
	/* Check if the new tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(locatorTool!=0)
		{
		BaseLocator* newLocator;
		if(algorithm==0)
			{
			/* Create a cutting plane locator object and associate it with the new tool: */
			newLocator=new CuttingPlaneLocator(locatorTool,this);
			}
		else if(algorithm<firstScalarAlgorithmIndex)
			{
			/* Create a scalar evaluation locator object and associate it with the new tool: */
			newLocator=new ScalarEvaluationLocator(locatorTool,this);
			}
		else if(algorithm<firstScalarAlgorithmIndex+module->getNumScalarAlgorithms())
			{
			/* Create a data locator object and associate it with the new tool: */
			int algorithmIndex=algorithm-firstScalarAlgorithmIndex;
			Visualization::Abstract::Algorithm* extractor=module->getScalarAlgorithm(algorithmIndex,colorMaps[scalarVariable],dataSet,scalarExtractor,Vrui::openPipe());
			newLocator=new DataLocator(locatorTool,this,module->getScalarAlgorithmName(algorithmIndex),extractor);
			}
		else if(algorithm<firstVectorAlgorithmIndex)
			{
			/* Create a vector evaluation locator object and associate it with the new tool: */
			newLocator=new VectorEvaluationLocator(locatorTool,this);
			}
		else
			{
			/* Create a data locator object and associate it with the new tool: */
			int algorithmIndex=algorithm-firstVectorAlgorithmIndex;
			Visualization::Abstract::Algorithm* extractor=module->getVectorAlgorithm(algorithmIndex,colorMaps[scalarVariable],dataSet,vectorExtractor,scalarExtractor,Vrui::openPipe());
			newLocator=new DataLocator(locatorTool,this,module->getScalarAlgorithmName(algorithmIndex),extractor);
			}
		
		/* Add new locator to list: */
		baseLocators.push_back(newLocator);
		}
	}

void Visualizer::toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData)
	{
	/* Check if the to-be-destroyed tool is a locator tool: */
	Vrui::LocatorTool* locatorTool=dynamic_cast<Vrui::LocatorTool*>(cbData->tool);
	if(locatorTool!=0)
		{
		/* Find the data locator associated with the tool in the list: */
		for(BaseLocatorList::iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
			if((*blIt)->getTool()==locatorTool)
				{
				/* Remove the locator: */
				delete *blIt;
				baseLocators.erase(blIt);
				break;
				}
		}
	}

void Visualizer::display(GLContextData& contextData) const
	{
	/* Highlight all locators: */
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->highlightLocator(contextData);
	
	/* Enable all cutting planes: */
	int numSupportedCuttingPlanes;
	glGetIntegerv(GL_MAX_CLIP_PLANES,&numSupportedCuttingPlanes);
	int cuttingPlaneIndex=0;
	for(size_t i=0;i<numCuttingPlanes&&cuttingPlaneIndex<numSupportedCuttingPlanes;++i)
		if(cuttingPlanes[i].active)
			{
			/* Enable the cutting plane: */
			glEnable(GL_CLIP_PLANE0+cuttingPlaneIndex);
			GLdouble cuttingPlane[4];
			for(int j=0;j<3;++j)
				cuttingPlane[j]=cuttingPlanes[i].plane.getNormal()[j];
			cuttingPlane[3]=-cuttingPlanes[i].plane.getOffset();
			glClipPlane(GL_CLIP_PLANE0+cuttingPlaneIndex,cuttingPlane);
			
			/* Go to the next cutting plane: */
			++cuttingPlaneIndex;
			}
	
	/* Render all opaque visualization elements: */
	for(ElementList::const_iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		if(veIt->show&&!veIt->element->usesTransparency())
			veIt->element->glRenderAction(contextData);
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->glRenderAction(contextData);

	/* Render all transparent visualization elements: */
	for(ElementList::const_iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		if(veIt->show&&veIt->element->usesTransparency())
			veIt->element->glRenderAction(contextData);
	for(BaseLocatorList::const_iterator blIt=baseLocators.begin();blIt!=baseLocators.end();++blIt)
		(*blIt)->glRenderActionTransparent(contextData);
	
	/* Render the data set: */
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH,&lineWidth);
	if(lineWidth!=1.0f)
		glLineWidth(1.0f);
	glColor(dataSetRenderColor);
	dataSetRenderer->glRenderAction(contextData);
	glLineWidth(lineWidth);
	
	/* Disable all cutting planes: */
	cuttingPlaneIndex=0;
	for(size_t i=0;i<numCuttingPlanes&&cuttingPlaneIndex<numSupportedCuttingPlanes;++i)
		if(cuttingPlanes[i].active)
			{
			/* Disable the cutting plane: */
			glDisable(GL_CLIP_PLANE0+cuttingPlaneIndex);
			
			/* Go to the next cutting plane: */
			++cuttingPlaneIndex;
			}
	}

void Visualizer::changeRenderingModeCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new rendering mode: */
	dataSetRenderer->setRenderingMode(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
	}

void Visualizer::changeScalarVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new scalar variable: */
	selectScalarVariable(cbData->radioBox->getToggleIndex(cbData->newSelectedToggle));
	}

void Visualizer::changeVectorVariableCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new vector variable: */
	vectorVariable=cbData->radioBox->getToggleIndex(cbData->newSelectedToggle);
	
	/* Get a new vector extractor: */
	delete vectorExtractor;
	vectorExtractor=dataSet->getVectorExtractor(vectorVariable);
	}

void Visualizer::changeAlgorithmCallback(GLMotif::RadioBox::ValueChangedCallbackData* cbData)
	{
	/* Set the new algorithm: */
	algorithm=cbData->radioBox->getToggleIndex(cbData->newSelectedToggle);
	}

void Visualizer::showElementListCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show element list based on toggle button state: */
	if(cbData->set)
		{
		if(elements.size()>0)
			{
			/* Pop up the element list at the same position as the main menu: */
			Vrui::getWidgetManager()->popupPrimaryWidget(elementListDialogPopup,Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
			}
		else
			cbData->toggle->setToggle(false);
		}
	else
		Vrui::popdownPrimaryWidget(elementListDialogPopup);
	}

void Visualizer::showElementSettingsDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	int rowIndex=elementListDialog->getChildIndex(cbData->toggle)/3;
	if(rowIndex>=0&&rowIndex<int(elements.size()))
		{
		if(cbData->set)
			{
			if(elements[rowIndex].settingsDialog!=0&&!elements[rowIndex].settingsDialogVisible)
				{
				Vrui::getWidgetManager()->popupPrimaryWidget(elements[rowIndex].settingsDialog,Vrui::getWidgetManager()->calcWidgetTransformation(cbData->toggle));
				elements[rowIndex].settingsDialogVisible=true;
				}
			else
				cbData->toggle->setToggle(false);
			}
		else
			{
			if(elements[rowIndex].settingsDialog!=0&&elements[rowIndex].settingsDialogVisible)
				{
				Vrui::popdownPrimaryWidget(elements[rowIndex].settingsDialog);
				elements[rowIndex].settingsDialogVisible=false;
				}
			}
		}
	else
		cbData->toggle->setToggle(false);
	}

void Visualizer::showElementCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	int rowIndex=elementListDialog->getChildIndex(cbData->toggle)/3;
	if(rowIndex>=0&&rowIndex<int(elements.size()))
		elements[rowIndex].show=cbData->set;
	else
		cbData->toggle->setToggle(false);
	}

void Visualizer::clearElementsCallback(Misc::CallbackData*)
	{
	/* Delete all finished visualization elements: */
	for(ElementList::iterator veIt=elements.begin();veIt!=elements.end();++veIt)
		{
		if(veIt->settingsDialogVisible)
			Vrui::popdownPrimaryWidget(veIt->settingsDialog);
		delete veIt->settingsDialog;
		}
	elements.clear();
	
	/* Reset the element list dialog: */
	Vrui::popdownPrimaryWidget(elementListDialogPopup);
	showElementListToggle->setToggle(false);
	delete elementListDialogPopup;
	elementListDialogPopup=createElementListDialog();
	}

void Visualizer::centerDisplayCallback(Misc::CallbackData*)
	{
	/* Get the data set's domain box: */
	DataSet::Box domain=dataSet->getDomainBox();
	Vrui::Point center=Geometry::mid(domain.getMin(),domain.getMax());
	Vrui::Scalar radius=Geometry::dist(domain.getMin(),domain.getMax());
	
	Vrui::setNavigationTransformation(center,radius);
	}

void Visualizer::showColorBarCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show color bar dialog based on toggle button state: */
	if(cbData->set)
		{
		/* Pop up the color bar dialog at the same position as the main menu: */
		Vrui::getWidgetManager()->popupPrimaryWidget(colorBarDialogPopup,Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
		}
	else
		Vrui::popdownPrimaryWidget(colorBarDialogPopup);
	}

void Visualizer::showPaletteEditorCallback(GLMotif::ToggleButton::ValueChangedCallbackData* cbData)
	{
	/* Hide or show palette editor based on toggle button state: */
	if(cbData->set)
		{
		/* Pop up the palette editor at the same position as the main menu: */
		Vrui::getWidgetManager()->popupPrimaryWidget(paletteEditor,Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
		}
	else
		Vrui::popdownPrimaryWidget(paletteEditor);
	}

void Visualizer::createStandardLuminancePaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData)
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
	switch(cbData->menu->getEntryIndex(cbData->selectedButton))
		{
		case 0: // Luminance palette grey
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case 1: // Luminance palette red
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(1.0f,0.287f,0.287f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case 2: // Luminance palette yellow
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.564f,0.564f,0.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case 3: // Luminance palette green
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.0f,0.852f,0.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case 4: // Luminance palette cyan
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.0f,0.713f,0.713f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case 5: // Luminance palette blue
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(0.436f,0.436f,1.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		
		case 6: // Luminance palette magenta
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.0f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(o+f*0.5,Color(1.0f,0.148f,1.0f,0.5f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,1.0f,1.0f,1.0f)));
			break;
		}
	
	/* Create the new color map: */
	paletteEditor->createPalette(controlPoints);
	}

void Visualizer::createStandardSaturationPaletteCallback(GLMotif::Menu::EntrySelectCallbackData* cbData)
	{
	/* Define local types: */
	typedef GLMotif::ColorMap ColorMap;
	typedef ColorMap::ValueRange ValueRange;
	typedef ColorMap::ColorMapValue Color;
	typedef ColorMap::ControlPoint ControlPoint;
	
	/* Get the current color map's value range: */
	const ValueRange& valueRange=paletteEditor->getColorMap()->getValueRange();
	
	/* Create a new control point vector for the current color map: */
	std::vector<ControlPoint> controlPoints;
	switch(cbData->menu->getEntryIndex(cbData->selectedButton))
		{
		case 0: // Saturation palette red -> cyan
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.0f,0.287f,0.287f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.0f,0.713f,0.713f,1.0f)));
			break;
		
		case 1: // Saturation palette yellow -> blue
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.564f,0.564f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.436f,0.436f,1.0f,1.0f)));
			break;
		
		case 2: // Saturation palette green -> magenta
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.852f,0.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,0.148f,1.0f,1.0f)));
			break;
		
		case 3: // Saturation palette cyan ->red
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.0f,0.713f,0.713f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(1.0f,0.287f,0.287f,1.0f)));
			break;
		
		case 4: // Saturation palette blue -> yellow
			controlPoints.push_back(ControlPoint(valueRange.first,Color(0.436f,0.436f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.564f,0.564f,0.0f,1.0f)));
			break;
		
		case 5: // Saturation palette magenta -> green
			controlPoints.push_back(ControlPoint(valueRange.first,Color(1.0f,0.148f,1.0f,0.0f)));
			controlPoints.push_back(ControlPoint(valueRange.second,Color(0.0f,0.852f,0.0f,1.0f)));
			break;
		
		case 6: // Rainbow palette
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
		}
	
	/* Create the new color map: */
	paletteEditor->createPalette(controlPoints);
	}

void Visualizer::savePaletteCallback(Misc::CallbackData* cbData)
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

void Visualizer::colorMapChangedCallback(Misc::CallbackData* cbData)
	{
	/* Export the changed palette to the current color map: */
	paletteEditor->exportColorMap(*colorMaps[scalarVariable]);
	
	Vrui::requestUpdate();
	}

int main(int argc,char* argv[])
	{
	try
		{
		char** appDefaults=0;
		Visualizer iso(argc,argv,appDefaults);
		iso.run();
		return 0;
		}
	catch(std::runtime_error err)
		{
		std::cerr<<"Caught exception "<<err.what()<<std::endl;
		return 1;
		}
	}
