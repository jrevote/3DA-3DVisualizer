/***********************************************************************
ScalarEvaluationLocator - Class for locators evaluating scalar
properties of data sets.
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

#include <Geometry/OrthogonalTransformation.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/TextField.h>
#include <GLMotif/RowColumn.h>
#include <Vrui/Vrui.h>

#include <Abstract/ScalarExtractor.h>
#include <Abstract/DataSet.h>

#include "PaletteEditor.h"

#include "Visualizer.h"

/****************************************************
Methods of class Visualizer::ScalarEvaluationLocator:
****************************************************/

Visualizer::ScalarEvaluationLocator::ScalarEvaluationLocator(Vrui::LocatorTool* sLocatorTool,Visualizer* sApplication)
	:EvaluationLocator(sLocatorTool,sApplication,"Scalar Evaluation Dialog"),
	 scalarExtractor(application->scalarExtractor->clone()),
	 valueValid(false)
	{
	/* Add to the evaluation dialog: */
	new GLMotif::Label("ValueLabel",evaluationDialog,application->dataSet->getScalarVariableName(application->scalarVariable));
	
	value=new GLMotif::TextField("Value",evaluationDialog,16);
	value->setPrecision(10);
	
	new GLMotif::Blind("Blind1",evaluationDialog);
	
	GLMotif::Button* insertControlPointButton=new GLMotif::Button("InsertControlPointButton",evaluationDialog,"Insert Color Map Control Point");
	insertControlPointButton->getSelectCallbacks().add(this,&Visualizer::ScalarEvaluationLocator::insertControlPointCallback);
	
	evaluationDialog->manageChild();
	
	/* Pop up the evaluation dialog: */
	Vrui::popupPrimaryWidget(evaluationDialogPopup,Vrui::getNavigationTransformation().transform(Vrui::getDisplayCenter()));
	}

Visualizer::ScalarEvaluationLocator::~ScalarEvaluationLocator(void)
	{
	/* Destroy the scalar extractor: */
	delete scalarExtractor;
	}

void Visualizer::ScalarEvaluationLocator::motionCallback(Vrui::LocatorTool::MotionCallbackData* cbData)
	{
	/* Call the base class method: */
	EvaluationLocator::motionCallback(cbData);
	
	if(dragging)
		{
		/* Get the current position of the locator in model coordinates: */
		point=locator->getPosition();
		
		/* Evaluate the data set at the locator's position: */
		if(locator->isValid())
			{
			valueValid=true;
			currentValue=locator->calcScalar(scalarExtractor);
			value->setValue(currentValue);
			}
		else
			{
			valueValid=false;
			value->setLabel("");
			}
		}
	}

void Visualizer::ScalarEvaluationLocator::insertControlPointCallback(Misc::CallbackData* cbData)
	{
	/* Insert a new control point into the color map: */
	if(valueValid)
		application->paletteEditor->getColorMap()->insertControlPoint(currentValue);
	}
