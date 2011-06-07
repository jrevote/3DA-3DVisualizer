/***********************************************************************
ArrowRakeExtractor - Wrapper class extract rakes of arrows from vector
fields.
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

#define VISUALIZATION_WRAPPERS_ARROWRAKEEXTRACTOR_IMPLEMENTATION

#include <Misc/ThrowStdErr.h>
#include <Math/Math.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Label.h>
#include <GLMotif/TextField.h>
#include <GLMotif/Slider.h>

#include <Wrappers/ScalarExtractor.h>
#include <Wrappers/VectorExtractor.h>
#include <Wrappers/ArrowRake.h>
#include <Wrappers/AlarmTimerElement.h>

#include <Wrappers/ArrowRakeExtractor.h>

namespace Visualization {

namespace Wrappers {

/***********************************
Methods of class ArrowRakeExtractor:
***********************************/

template <class DataSetWrapperParam>
inline
const typename ArrowRakeExtractor<DataSetWrapperParam>::DS*
ArrowRakeExtractor<DataSetWrapperParam>::getDs(
	const Visualization::Abstract::DataSet* sDataSet)
	{
	/* Get a pointer to the data set wrapper: */
	const DataSetWrapper* myDataSet=dynamic_cast<const DataSetWrapper*>(sDataSet);
	if(myDataSet==0)
		Misc::throwStdErr("ArrowRakeExtractor::ArrowRakeExtractor: Mismatching data set type");
	
	return &myDataSet->getDs();
	}

template <class DataSetWrapperParam>
inline
const typename ArrowRakeExtractor<DataSetWrapperParam>::VE&
ArrowRakeExtractor<DataSetWrapperParam>::getVe(
	const Visualization::Abstract::VectorExtractor* sVectorExtractor)
	{
	/* Get a pointer to the vector extractor wrapper: */
	const VectorExtractor* myVectorExtractor=dynamic_cast<const VectorExtractor*>(sVectorExtractor);
	if(myVectorExtractor==0)
		Misc::throwStdErr("ArrowRakeExtractor::ArrowRakeExtractor: Mismatching vector extractor type");
	
	return myVectorExtractor->getVe();
	}

template <class DataSetWrapperParam>
inline
const typename ArrowRakeExtractor<DataSetWrapperParam>::SE&
ArrowRakeExtractor<DataSetWrapperParam>::getSe(
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor)
	{
	/* Get a pointer to the scalar extractor wrapper: */
	const ScalarExtractor* myScalarExtractor=dynamic_cast<const ScalarExtractor*>(sScalarExtractor);
	if(myScalarExtractor==0)
		Misc::throwStdErr("ArrowRakeExtractor::ArrowRakeExtractor: Mismatching scalar extractor type");
	
	return myScalarExtractor->getSe();
	}

template <class DataSetWrapperParam>
inline
ArrowRakeExtractor<DataSetWrapperParam>::ArrowRakeExtractor(
	const GLColorMap* sColorMap,
	const Visualization::Abstract::DataSet* sDataSet,
	const Visualization::Abstract::VectorExtractor* sVectorExtractor,
	const Visualization::Abstract::ScalarExtractor* sScalarExtractor,
	Comm::MulticastPipe* sPipe)
	:Abstract::Algorithm(sPipe),
	 colorMap(sColorMap),
	 dataSet(getDs(sDataSet)),
	 vectorExtractor(getVe(sVectorExtractor)),
	 scalarExtractor(getSe(sScalarExtractor)),
	 rakeSize(5,5),
	 baseCellSize(dataSet->calcAverageCellSize()),
	 lengthScale(1),
	 currentArrowRake(0)
	{
	/* Initialize the rake cell size: */
	for(int i=0;i<2;++i)
		cellSize[i]=baseCellSize;
	}

template <class DataSetWrapperParam>
inline
ArrowRakeExtractor<DataSetWrapperParam>::~ArrowRakeExtractor(
	void)
	{
	}

template <class DataSetWrapperParam>
inline
bool
ArrowRakeExtractor<DataSetWrapperParam>::hasSeededCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
bool
ArrowRakeExtractor<DataSetWrapperParam>::hasIncrementalCreator(
	void) const
	{
	return true;
	}

template <class DataSetWrapperParam>
inline
GLMotif::Widget*
ArrowRakeExtractor<DataSetWrapperParam>::createSettingsDialog(
	GLMotif::WidgetManager* widgetManager)
	{
	/* Get the style sheet: */
	const GLMotif::StyleSheet* ss=widgetManager->getStyleSheet();
	
	/* Create the settings dialog window: */
	GLMotif::PopupWindow* settingsDialogPopup=new GLMotif::PopupWindow("ArrowRakeExtractorSettingsDialogPopup",widgetManager,"Arrow Rake Extractor Settings");
	
	GLMotif::RowColumn* settingsDialog=new GLMotif::RowColumn("settingsDialog",settingsDialogPopup,false);
	settingsDialog->setNumMinorWidgets(3);
	
	for(int i=0;i<2;++i)
		{
		new GLMotif::Label("RakeSizeLabel",settingsDialog,i==0?"Rake Width":"Rake Height");
		
		rakeSizeValues[i]=new GLMotif::TextField("RakeSizeValue",settingsDialog,6);
		rakeSizeValues[i]->setValue(rakeSize[i]);
		
		rakeSizeSliders[i]=new GLMotif::Slider("RakeSizeSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
		rakeSizeSliders[i]->setValueRange(1,100,1);
		rakeSizeSliders[i]->setValue(rakeSize[i]);
		rakeSizeSliders[i]->getValueChangedCallbacks().add(this,&ArrowRakeExtractor::rakeSizeSliderCallback);
		}
	
	for(int i=0;i<2;++i)
		{
		new GLMotif::Label("CellSizeLabel",settingsDialog,i==0?"Cell Width":"Cell Height");
		
		cellSizeValues[i]=new GLMotif::TextField("CellSizeValue",settingsDialog,6);
		cellSizeValues[i]->setValue(cellSize[i]);
		
		cellSizeSliders[i]=new GLMotif::Slider("CellSizeSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
		cellSizeSliders[i]->setValueRange(-4.0,4.0,0.1);
		cellSizeSliders[i]->setValue(Math::log10(cellSize[i]/baseCellSize));
		cellSizeSliders[i]->getValueChangedCallbacks().add(this,&ArrowRakeExtractor::cellSizeSliderCallback);
		}
	
	new GLMotif::Label("LengthScaleLabel",settingsDialog,"Arrow Scale");
	
	lengthScaleValue=new GLMotif::TextField("LengthScaleValue",settingsDialog,12);
	lengthScaleValue->setPrecision(6);
	lengthScaleValue->setValue(lengthScale);
	
	lengthScaleSlider=new GLMotif::Slider("LengthScaleSlider",settingsDialog,GLMotif::Slider::HORIZONTAL,ss->fontHeight*10.0f);
	lengthScaleSlider->setValueRange(-4.0,4.0,0.1);
	lengthScaleSlider->setValue(Math::log10(lengthScale));
	lengthScaleSlider->getValueChangedCallbacks().add(this,&ArrowRakeExtractor::lengthScaleSliderCallback);
	
	settingsDialog->manageChild();
	
	return settingsDialogPopup;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::createElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Create the rake frame: */
	Point base=Point(seedLocator->getPosition());
	Vector x=Vector(seedLocator->getOrientation().getDirection(0));
	base-=x*Math::div2(Scalar(rakeSize[0]))*cellSize[0];
	Vector y=Vector(seedLocator->getOrientation().getDirection(2));
	base-=y*Math::div2(Scalar(rakeSize[1]))*cellSize[1];
	
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("ArrowRakeExtractor::createElement: Mismatching locator type");
	const DSL& dsl=myLocator->getDsl();
	
	/* Create a new arrow rake visualization element: */
	ArrowRake* result=new ArrowRake(colorMap,rakeSize,getPipe());
	
	/* Calculate the arrow base points and directions: */
	for(typename Rake::Index index(0);index[0]<rakeSize[0];index.preInc(rakeSize))
		{
		result->getRake()(index).base=base;
		result->getRake()(index).base+=x*(Scalar(index[0])*cellSize[0]);
		result->getRake()(index).base+=y*(Scalar(index[1])*cellSize[1]);
		DSL locator=dsl;
		if((result->getRake()(index).valid=locator.locatePoint(result->getRake()(index).base)))
			{
			result->getRake()(index).direction=Vector(locator.calcValue(vectorExtractor))*lengthScale;
			result->getRake()(index).scalarValue=Scalar(locator.calcValue(scalarExtractor));
			}
		}
	result->update();
	
	/* Return the result: */
	return result;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::startElement(
	const Visualization::Abstract::DataSet::Locator* seedLocator)
	{
	/* Create the rake frame: */
	currentBase=Point(seedLocator->getPosition());
	currentX=Vector(seedLocator->getOrientation().getDirection(0));
	currentBase-=currentX*Math::div2(Scalar(rakeSize[0]))*cellSize[0];
	currentY=Vector(seedLocator->getOrientation().getDirection(2));
	currentBase-=currentY*Math::div2(Scalar(rakeSize[1]))*cellSize[1];
	
	/* Get a pointer to the locator wrapper: */
	const Locator* myLocator=dynamic_cast<const Locator*>(seedLocator);
	if(myLocator==0)
		Misc::throwStdErr("ArrowRakeExtractor::createElement: Mismatching locator type");
	currentDsl=myLocator->getDsl();
	
	/* Create a new slice visualization element: */
	currentArrowRake=new ArrowRake(colorMap,rakeSize,getPipe());
	
	/* Return the result: */
	return currentArrowRake.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
ArrowRakeExtractor<DataSetWrapperParam>::continueElement(
	const Realtime::AlarmTimer& alarm)
	{
	/* Calculate the arrow base points and directions: */
	for(typename Rake::Index index(0);index[0]<rakeSize[0];index.preInc(rakeSize))
		{
		currentArrowRake->getRake()(index).base=currentBase;
		currentArrowRake->getRake()(index).base+=currentX*(Scalar(index[0])*cellSize[0]);
		currentArrowRake->getRake()(index).base+=currentY*(Scalar(index[1])*cellSize[1]);
		DSL locator=currentDsl;
		if(locator.locatePoint(currentArrowRake->getRake()(index).base))
			{
			currentArrowRake->getRake()(index).direction=Vector(locator.calcValue(vectorExtractor))*lengthScale;
			currentArrowRake->getRake()(index).scalarValue=Scalar(locator.calcValue(scalarExtractor));
			currentArrowRake->getRake()(index).valid=true;
			}
		else
			currentArrowRake->getRake()(index).valid=false;
		}
	currentArrowRake->update();
	return true;
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::finishElement(
	void)
	{
	currentArrowRake=0;
	}

template <class DataSetWrapperParam>
inline
Visualization::Abstract::Element*
ArrowRakeExtractor<DataSetWrapperParam>::startSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("ArrowRakeExtractor::startSlaveElement: Cannot be called on master node");
	
	/* Create a new arrow rake visualization element: */
	currentArrowRake=new ArrowRake(colorMap,rakeSize,getPipe());
	
	return currentArrowRake.getPointer();
	}

template <class DataSetWrapperParam>
inline
bool
ArrowRakeExtractor<DataSetWrapperParam>::continueSlaveElement(
	void)
	{
	if(getPipe()==0||getPipe()->isMaster())
		Misc::throwStdErr("ArrowRakeExtractor::continueSlaveElement: Cannot be called on master node");
	
	return true;
	// return currentArrowRake->receive();
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::rakeSizeSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	int dimension=cbData->slider==rakeSizeSliders[0]?0:1;
	
	/* Get the new slider value: */
	rakeSize[dimension]=int(Math::floor(double(cbData->value)+0.5));
	
	/* Update the text field: */
	rakeSizeValues[dimension]->setValue(rakeSize[dimension]);
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::cellSizeSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	int dimension=cbData->slider==cellSizeSliders[0]?0:1;
	
	/* Get the new slider value and convert to cell size: */
	cellSize[dimension]=Scalar(Math::pow(10.0,double(cbData->value)))*baseCellSize;
	
	/* Update the text field: */
	cellSizeValues[dimension]->setValue(cellSize[dimension]);
	}

template <class DataSetWrapperParam>
inline
void
ArrowRakeExtractor<DataSetWrapperParam>::lengthScaleSliderCallback(
	GLMotif::Slider::ValueChangedCallbackData* cbData)
	{
	/* Get the new slider value and convert to cell size: */
	lengthScale=Scalar(Math::pow(10.0,double(cbData->value)));
	
	/* Update the text field: */
	lengthScaleValue->setValue(lengthScale);
	}

}

}
