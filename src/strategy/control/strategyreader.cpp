/***************************************************************************
 *   Copyright (C) 2008 by A.R. Offringa   *
 *   offringa@astro.rug.nl   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "strategyreader.h"

#include "../../util/numberparser.h"

#include "../actions/absthresholdaction.h"
#include "../actions/action.h"
#include "../actions/adapter.h"
#include "../actions/addstatisticsaction.h"
#include "../actions/baselineselectionaction.h"
#include "../actions/calibratepassbandaction.h"
#include "../actions/changeresolutionaction.h"
#include "../actions/collectnoisestatisticsaction.h"
#include "../actions/combineflagresultsaction.h"
#include "../actions/cutareaaction.h"
#include "../actions/directionalcleanaction.h"
#include "../actions/directionprofileaction.h"
#include "../actions/dumpimagesaction.h"
#include "../actions/eigenvalueverticalaction.h"
#include "../actions/foreachbaselineaction.h"
#include "../actions/foreachcomplexcomponentaction.h"
#include "../actions/foreachmsaction.h"
#include "../actions/foreachpolarisationaction.h"
#include "../actions/fouriertransformaction.h"
#include "../actions/frequencyconvolutionaction.h"
#include "../actions/frequencyselectionaction.h"
#include "../actions/fringestopaction.h"
#include "../actions/highpassfilteraction.h"
#include "../actions/imageraction.h"
#include "../actions/iterationaction.h"
#include "../actions/normalizevarianceaction.h"
#include "../actions/plotaction.h"
#include "../actions/quickcalibrateaction.h"
#include "../actions/rawappenderaction.h"
#include "../actions/setflaggingaction.h"
#include "../actions/setimageaction.h"
#include "../actions/slidingwindowfitaction.h"
#include "../actions/statisticalflagaction.h"
#include "../actions/strategyaction.h"
#include "../actions/svdaction.h"
#include "../actions/sumthresholdaction.h"
#include "../actions/timeconvolutionaction.h"
#include "../actions/timeselectionaction.h"
#include "../actions/uvprojectaction.h"
#include "../actions/writedataaction.h"
#include "../actions/writeflagsaction.h"

#define ENCODING "UTF-8"

namespace rfiStrategy {

int StrategyReader::useCount = 0;

StrategyReader::StrategyReader()
{
	if(useCount == 0)
	{
		LIBXML_TEST_VERSION ;
	}
	++useCount;
}


StrategyReader::~StrategyReader()
{
	if(useCount == 0)
		xmlCleanupParser();
}

Strategy *StrategyReader::CreateStrategyFromFile(const std::string &filename)
{
	_xmlDocument = xmlReadFile(filename.c_str(), NULL, 0);
	if (_xmlDocument == NULL)
		throw StrategyReaderError("Failed to read file");

	xmlNode *rootElement = xmlDocGetRootElement(_xmlDocument);
	Strategy *strategy = 0;

	for (xmlNode *curNode=rootElement; curNode!=NULL; curNode=curNode->next)
	{
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(strategy != 0)
				throw StrategyReaderError("Multiple root elements found.");
			if(std::string((const char *) curNode->name) != "rfi-strategy")
				throw StrategyReaderError("Invalid structure in xml file: no rfi-strategy root node found. Maybe this is not an rfi strategy?");
			
			xmlChar *formatVersionCh = xmlGetProp(curNode, BAD_CAST "format-version");
			if(formatVersionCh == 0)
				throw StrategyReaderError("Missing attribute 'format-version'");
			double formatVersion = NumberParser::ToDouble((const char*) formatVersionCh);
			xmlFree(formatVersionCh);

			xmlChar *readerVersionRequiredCh = xmlGetProp(curNode, BAD_CAST "reader-version-required");
			if(readerVersionRequiredCh == 0)
				throw StrategyReaderError("Missing attribute 'reader-version-required'");
			double readerVersionRequired = NumberParser::ToDouble((const char*) readerVersionRequiredCh);
			xmlFree(readerVersionRequiredCh);
			
			if(readerVersionRequired > STRATEGY_FILE_FORMAT_VERSION)
				throw StrategyReaderError("This file requires a newer software version");
			if(formatVersion < STRATEGY_FILE_FORMAT_VERSION_REQUIRED)
			{
				std::stringstream s;
				s << "This file is too old for the software, please recreate the strategy. File format version: " << formatVersion << ", oldest version that this software understands: " << STRATEGY_FILE_FORMAT_VERSION_REQUIRED << " (these versions are numbered differently from the software).";
				throw StrategyReaderError(s.str());
			}
			
			strategy = parseRootChildren(curNode);
		}
	}
	if(strategy == 0)
		throw StrategyReaderError("Could not find root element in file.");

	xmlFreeDoc(_xmlDocument);

	return strategy;
}

Strategy *StrategyReader::parseRootChildren(xmlNode *rootNode)
{
	Strategy *strategy = 0;
	for (xmlNode *curNode=rootNode->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			if(strategy != 0)
				throw StrategyReaderError("More than one root element in file!");
			strategy = dynamic_cast<Strategy*>(parseAction(curNode));
			if(strategy == 0)
				throw StrategyReaderError("Root element was not a strategy!");
		}
	}
	if(strategy == 0)
		throw StrategyReaderError("Root element not found.");

	return strategy;
}

Action *StrategyReader::parseChild(xmlNode *node)
{
	if (node->type == XML_ELEMENT_NODE) {
		std::string name((const char*) node->name);
		if(name == "action")
			return parseAction(node);
	}
	throw StrategyReaderError("Invalid structure in xml file: an action was expected");
}

class Strategy *StrategyReader::parseStrategy(xmlNode *node)
{
	Strategy *strategy = new Strategy();
	parseChildren(node, strategy);
	return strategy;
}

void StrategyReader::parseChildren(xmlNode *node, ActionContainer *parent) 
{
	for (xmlNode *curOuterNode=node->children; curOuterNode!=NULL; curOuterNode=curOuterNode->next) {
		if(curOuterNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curOuterNode->name);
			if(nameStr == "children")
			{
				for (xmlNode *curNode=curOuterNode->children; curNode!=NULL; curNode=curNode->next) {
					if (curNode->type == XML_ELEMENT_NODE) {
						parent->Add(parseChild(curNode));
					}
				}
			}
		}
	}
}

xmlNode *StrategyReader::getTextNode(xmlNode *node, const char *subNodeName, bool allowEmpty) const 
{
	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curNode->name);
			if(nameStr == subNodeName)
			{
				curNode = curNode->children;
				if(curNode == 0 || curNode->type != XML_TEXT_NODE)
				{
					if(allowEmpty)
						return 0;
					else
						throw StrategyReaderError("Error occured in reading xml file: value node did not contain text");
				}
				return curNode;
			}
		}
	}
	std::ostringstream str;
	str << "Error occured in reading xml file: could not find value node \"" << subNodeName << '\"';
	throw StrategyReaderError(str.str());
}

int StrategyReader::getInt(xmlNode *node, const char *name) const 
{
	xmlNode *valNode = getTextNode(node, name);
	return atoi((const char *) valNode->content);
}

double StrategyReader::getDouble(xmlNode *node, const char *name) const 
{
	xmlNode *valNode = getTextNode(node, name);
	return NumberParser::ToDouble((const char *) valNode->content);
}

std::string StrategyReader::getString(xmlNode *node, const char *name) const 
{
	xmlNode *valNode = getTextNode(node, name, true);
	if(valNode == 0)
		return std::string();
	else
		return std::string((const char *) valNode->content);
}

Action *StrategyReader::parseAction(xmlNode *node)
{
	Action *newAction = 0;
	xmlChar *typeCh = xmlGetProp(node, BAD_CAST "type");
	if(typeCh == 0)
		throw StrategyReaderError("Action tag did not have 'type' parameter");
	std::string typeStr((const char*) typeCh);
	if(typeStr == "AbsThresholdAction")
		newAction = parseAbsThresholdAction(node);
	else if(typeStr == "Adapter")
		newAction = parseAdapter(node);
	else if(typeStr == "AddStatisticsAction")
		newAction = parseAddStatistics(node);
	else if(typeStr == "BaselineSelectionAction")
		newAction = parseBaselineSelectionAction(node);
	else if(typeStr == "CalibratePassbandAction")
		newAction = parseCalibratePassbandAction(node);
	else if(typeStr == "ChangeResolutionAction")
		newAction = parseChangeResolutionAction(node);
	else if(typeStr == "CollectNoiseStatisticsAction")
		newAction = parseCollectNoiseStatisticsAction(node);
	else if(typeStr == "CombineFlagResults")
		newAction = parseCombineFlagResults(node);
	else if(typeStr == "CutAreaAction")
		newAction = parseCutAreaAction(node);
	else if(typeStr == "DirectionalCleanAction")
		newAction = parseDirectionalCleanAction(node);
	else if(typeStr == "DirectionProfileAction")
		newAction = parseDirectionProfileAction(node);
	else if(typeStr == "DumpImagesAction")
		newAction = parseDumpImagesAction(node);
	else if(typeStr == "EigenValueVerticalAction")
		newAction = parseEigenValueVerticalAction(node);
	else if(typeStr == "ForEachBaselineAction")
		newAction = parseForEachBaselineAction(node);
	else if(typeStr == "ForEachComplexComponentAction")
		newAction = parseForEachComplexComponentAction(node);
	else if(typeStr == "ForEachMSAction")
		newAction = parseForEachMSAction(node);
	else if(typeStr == "ForEachPolarisationBlock")
		newAction = parseForEachPolarisationBlock(node);
	else if(typeStr == "FourierTransformAction")
		newAction = parseFourierTransformAction(node);
	else if(typeStr == "FrequencyConvolutionAction")
		newAction = parseFrequencyConvolutionAction(node);
	else if(typeStr == "FrequencySelectionAction")
		newAction = parseFrequencySelectionAction(node);
	else if(typeStr == "FringeStopAction")
		newAction = parseFringeStopAction(node);
	else if(typeStr == "HighPassFilterAction")
		newAction = parseHighPassFilterAction(node);
	else if(typeStr == "ImagerAction")
		newAction = parseImagerAction(node);
	else if(typeStr == "IterationBlock")
		newAction = parseIterationBlock(node);
	else if(typeStr == "NormalizeVarianceAction")
		newAction = parseNormalizeVarianceAction(node);
	else if(typeStr == "PlotAction")
		newAction = parsePlotAction(node);
	else if(typeStr == "QuickCalibrateAction")
		newAction = parseQuickCalibrateAction(node);
	else if(typeStr == "RawAppenderAction")
		newAction = parseRawAppenderAction(node);
	else if(typeStr == "SetFlaggingAction")
		newAction = parseSetFlaggingAction(node);
	else if(typeStr == "SetImageAction")
		newAction = parseSetImageAction(node);
	else if(typeStr == "SlidingWindowFitAction")
		newAction = parseSlidingWindowFitAction(node);
	else if(typeStr == "StatisticalFlagAction")
		newAction = parseStatisticalFlagAction(node);
	else if(typeStr == "SVDAction")
		newAction = parseSVDAction(node);
	else if(typeStr == "Strategy")
		newAction = parseStrategy(node);
	else if(typeStr == "SumThresholdAction")
		newAction = parseSumThresholdAction(node);
	else if(typeStr == "TimeConvolutionAction")
		newAction = parseTimeConvolutionAction(node);
	else if(typeStr == "TimeSelectionAction")
		newAction = parseTimeSelectionAction(node);
	else if(typeStr == "UVProjectAction")
		newAction = parseUVProjectAction(node);
	else if(typeStr == "WriteDataAction")
		newAction = parseWriteDataAction(node);
	else if(typeStr == "WriteFlagsAction")
		newAction = parseWriteFlagsAction(node);
	xmlFree(typeCh);
	if(newAction == 0)
	{
		std::stringstream s;
		s << "Unknown action type '" << typeStr << "' in xml file";
		throw StrategyReaderError(s.str());
	}
	return newAction;
}

Action *StrategyReader::parseAbsThresholdAction(xmlNode *node)
{
	AbsThresholdAction *newAction = new AbsThresholdAction();
	newAction->SetThreshold(getDouble(node, "threshold"));
	return newAction;
}

Action *StrategyReader::parseAdapter(xmlNode *node)
{
	Adapter *newAction = new Adapter();
	newAction->SetRestoreOriginals(getBool(node, "restore-originals"));
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseAddStatistics(xmlNode *node)
{
	AddStatisticsAction *newAction = new AddStatisticsAction();
	newAction->SetFilePrefix(getString(node, "file-prefix"));
	newAction->SetCompareOriginalAndAlternative(getBool(node, "compare-original-and-alternative"));
	newAction->SetSeparateBaselineStatistics(getBool(node, "separate-baseline-statistics"));
	newAction->SetPerformClassification(getBool(node, "perform-classification"));
	newAction->SetWriteImmediately(getBool(node, "write-immediately"));
	return newAction;
}

Action *StrategyReader::parseBaselineSelectionAction(xmlNode *node)
{
	BaselineSelectionAction *newAction = new BaselineSelectionAction();
	newAction->SetPreparationStep(getBool(node, "preparation-step"));
	newAction->SetFlagBadBaselines(getBool(node, "flag-bad-baselines"));
	newAction->SetThreshold(getDouble(node, "threshold"));
	newAction->SetAbsThreshold(getDouble(node, "abs-threshold"));
	newAction->SetSmoothingSigma(getDouble(node, "smoothing-sigma"));
	newAction->SetMakePlot(getBool(node, "make-plot"));
	return newAction;
}

Action *StrategyReader::parseCalibratePassbandAction(xmlNode *node)
{
	CalibratePassbandAction *newAction = new CalibratePassbandAction();
	newAction->SetSteps(getInt(node, "steps"));
	return newAction;
}

Action *StrategyReader::parseChangeResolutionAction(xmlNode *node)
{
	ChangeResolutionAction *newAction = new ChangeResolutionAction();
	newAction->SetTimeDecreaseFactor(getInt(node, "time-decrease-factor"));
	newAction->SetFrequencyDecreaseFactor(getInt(node, "frequency-decrease-factor"));
	newAction->SetRestoreRevised(getBool(node, "restore-revised"));
	newAction->SetRestoreMasks(getBool(node, "restore-masks"));
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseCollectNoiseStatisticsAction(xmlNode *node)
{
	CollectNoiseStatisticsAction *newAction = new CollectNoiseStatisticsAction();
	newAction->SetFilename(getString(node, "filename"));
	newAction->SetChannelDistance(getInt(node, "channel-distance"));
	newAction->SetTileWidth(getInt(node, "tile-timestep-size"));
	newAction->SetTileHeight(getInt(node, "tile-channels-size"));
	return newAction;
}

Action *StrategyReader::parseCombineFlagResults(xmlNode *node)
{
	CombineFlagResults *newAction = new CombineFlagResults();
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseCutAreaAction(xmlNode *node)
{
	CutAreaAction *newAction = new CutAreaAction();
	newAction->SetStartTimeSteps(getInt(node, "start-time-steps"));
	newAction->SetEndTimeSteps(getInt(node, "end-time-steps"));
	newAction->SetTopChannels(getInt(node, "top-channels"));
	newAction->SetBottomChannels(getInt(node, "bottom-channels"));
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseDirectionalCleanAction(xmlNode *node)
{
	DirectionalCleanAction *newAction = new DirectionalCleanAction();
	newAction->SetLimitingDistance(getDouble(node, "limiting-distance"));
	newAction->SetChannelConvolutionSize(getInt(node, "channel-convolution-size"));
	newAction->SetAttenuationOfCenter(getDouble(node, "attenuation-of-center"));
	newAction->SetMakePlot(getBool(node, "make-plot"));
	return newAction;
}

Action *StrategyReader::parseDirectionProfileAction(xmlNode *node)
{
	DirectionProfileAction *newAction = new DirectionProfileAction();
	newAction->SetAxis((enum DirectionProfileAction::Axis) getInt(node, "axis"));
	newAction->SetProfileAction((enum DirectionProfileAction::ProfileAction) getInt(node, "profile-action"));
	return newAction;
}

Action *StrategyReader::parseDumpImagesAction(xmlNode *node)
{
	DumpImagesAction *newAction = new DumpImagesAction();
	newAction->SetIdent(getInt(node, "ident"));
	return newAction;
}

Action *StrategyReader::parseEigenValueVerticalAction(xmlNode *)
{
	EigenValueVerticalAction *newAction = new EigenValueVerticalAction();
	return newAction;
}

Action *StrategyReader::parseForEachBaselineAction(xmlNode *node)
{
	ForEachBaselineAction *newAction = new ForEachBaselineAction();
	newAction->SetSelection((BaselineSelection) getInt(node, "selection"));
	newAction->SetThreadCount(getInt(node, "thread-count"));

	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curNode->name);
			if(nameStr == "antennae-to-skip")
			{
				for (xmlNode *curNode2=curNode->children; curNode2!=NULL; curNode2=curNode2->next) {
					if (curNode2->type == XML_ELEMENT_NODE) {
						std::string innerNameStr((const char *) curNode2->name);
						if(innerNameStr != "antenna")
							throw StrategyReaderError("Format of the for each baseline action is incorrect");
						xmlNode *textNode = curNode2->children;
						if(textNode->type != XML_TEXT_NODE)
							throw StrategyReaderError("Error occured in reading xml file: value node did not contain text");
						if(textNode->content != NULL)
						{
							newAction->AntennaeToSkip().insert(atoi((const char *) textNode->content));
						}
					}
				}
			}
			if(nameStr == "antennae-to-include")
			{
				for (xmlNode *curNode2=curNode->children; curNode2!=NULL; curNode2=curNode2->next) {
					if (curNode2->type == XML_ELEMENT_NODE) {
						std::string innerNameStr((const char *) curNode2->name);
						if(innerNameStr != "antenna")
							throw StrategyReaderError("Format of the for each baseline action is incorrect");
						xmlNode *textNode = curNode2->children;
						if(textNode->type != XML_TEXT_NODE)
							throw StrategyReaderError("Error occured in reading xml file: value node did not contain text");
						if(textNode->content != NULL)
						{
							newAction->AntennaeToInclude().insert(atoi((const char *) textNode->content));
						}
					}
				}
			}
		}
	}
	
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseForEachComplexComponentAction(xmlNode *node)
{
	ForEachComplexComponentAction *newAction = new ForEachComplexComponentAction();
	newAction->SetOnAmplitude(getBool(node, "on-amplitude"));
	newAction->SetOnPhase(getBool(node, "on-phase"));
	newAction->SetOnReal(getBool(node, "on-real"));
	newAction->SetOnImaginary(getBool(node, "on-imaginary"));
	newAction->SetRestoreFromAmplitude(getBool(node, "restore-from-amplitude"));
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseForEachMSAction(xmlNode *node)
{
	ForEachMSAction *newAction = new ForEachMSAction();
	newAction->SetDataColumnName(getString(node, "data-column-name"));
	newAction->SetSubtractModel(getBool(node, "subtract-model"));

	for (xmlNode *curNode=node->children; curNode!=NULL; curNode=curNode->next) {
		if(curNode->type == XML_ELEMENT_NODE)
		{
			std::string nameStr((const char *) curNode->name);
			if(nameStr == "filenames")
			{
				for (xmlNode *curNode2=curNode->children; curNode2!=NULL; curNode2=curNode2->next) {
					if (curNode2->type == XML_ELEMENT_NODE) {
						std::string innerNameStr((const char *) curNode2->name);
						if(innerNameStr != "filename")
							throw StrategyReaderError("Format of the for each MS action is incorrect");
						newAction->Filenames().push_back((const char *) curNode2->content);
					}
				}
			}
		}
	}
	
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseForEachPolarisationBlock(xmlNode *node)
{
	ForEachPolarisationBlock *newAction = new ForEachPolarisationBlock();
	newAction->SetOnXX(getBool(node, "on-xx"));
	newAction->SetOnXY(getBool(node, "on-xy"));
	newAction->SetOnYX(getBool(node, "on-yx"));
	newAction->SetOnYY(getBool(node, "on-yy"));
	newAction->SetOnStokesI(getBool(node, "on-stokes-i"));
	newAction->SetOnStokesQ(getBool(node, "on-stokes-q"));
	newAction->SetOnStokesU(getBool(node, "on-stokes-u"));
	newAction->SetOnStokesV(getBool(node, "on-stokes-v"));
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseFourierTransformAction(xmlNode *)
{
	FourierTransformAction *newAction = new FourierTransformAction();
	return newAction;
}

Action *StrategyReader::parseFrequencyConvolutionAction(xmlNode *node)
{
	FrequencyConvolutionAction *newAction = new FrequencyConvolutionAction();
	newAction->SetConvolutionSize(getInt(node, "convolution-size"));
	newAction->SetKernelKind((enum FrequencyConvolutionAction::KernelKind) getInt(node, "kernel-kind"));
	return newAction;
}

Action *StrategyReader::parseFrequencySelectionAction(xmlNode *node)
{
	FrequencySelectionAction *newAction = new FrequencySelectionAction();
	newAction->SetThreshold(getDouble(node, "threshold"));
	return newAction;
}

Action *StrategyReader::parseFringeStopAction(xmlNode *node)
{
	FringeStopAction *newAction = new FringeStopAction();
	newAction->SetFitChannelsIndividually(getBool(node, "fit-channels-individually"));
	newAction->SetFringesToConsider(getDouble(node, "fringes-to-consider"));
	newAction->SetOnlyFringeStop(getBool(node, "only-fringe-stop"));
	newAction->SetMinWindowSize(getInt(node, "min-window-size"));
	newAction->SetMaxWindowSize(getInt(node, "max-window-size"));
	return newAction;
}

Action *StrategyReader::parseHighPassFilterAction(xmlNode *node)
{
	HighPassFilterAction *newAction = new HighPassFilterAction();
	newAction->SetHKernelSigmaSq(getDouble(node, "horizontal-kernel-sigma-sq"));
	newAction->SetVKernelSigmaSq(getDouble(node, "vertical-kernel-sigma-sq"));
	newAction->SetWindowWidth(getInt(node, "window-width"));
	newAction->SetWindowHeight(getInt(node, "window-height"));
	newAction->SetMode((enum HighPassFilterAction::Mode) getInt(node, "mode"));
	return newAction;
}

Action *StrategyReader::parseImagerAction(xmlNode *)
{
	ImagerAction *newAction = new ImagerAction();
	return newAction;
}

Action *StrategyReader::parseIterationBlock(xmlNode *node)
{
	IterationBlock *newAction = new IterationBlock();
	newAction->SetIterationCount(getInt(node, "iteration-count"));
	newAction->SetSensitivityStart(getDouble(node, "sensitivity-start"));
	parseChildren(node, newAction);
	return newAction;
}

Action *StrategyReader::parseNormalizeVarianceAction(xmlNode *node)
{
	NormalizeVarianceAction *newAction = new NormalizeVarianceAction();
	newAction->SetMedianFilterSizeInS(getDouble(node, "median-filter-size-in-s"));
	return newAction;
}

Action *StrategyReader::parseSetFlaggingAction(xmlNode *node)
{
	SetFlaggingAction *newAction = new SetFlaggingAction();
	newAction->SetNewFlagging((enum SetFlaggingAction::NewFlagging) getInt(node, "new-flagging"));
	return newAction;
}

Action *StrategyReader::parsePlotAction(xmlNode *node)
{
	PlotAction *newAction = new PlotAction();
	newAction->SetPlotKind((enum PlotAction::PlotKind) getInt(node, "plot-kind"));
	newAction->SetLogarithmicYAxis(getBool(node, "logarithmic-y-axis"));
	return newAction;
}

Action *StrategyReader::parseQuickCalibrateAction(xmlNode *)
{
	QuickCalibrateAction *newAction = new QuickCalibrateAction();
	return newAction;
}

Action *StrategyReader::parseRawAppenderAction(xmlNode *)
{
	RawAppenderAction *newAction = new RawAppenderAction();
	return newAction;
}

class Action *StrategyReader::parseSetImageAction(xmlNode *node)
{
	SetImageAction *newAction = new SetImageAction();
	newAction->SetNewImage((enum SetImageAction::NewImage) getInt(node, "new-image"));
	return newAction;
}

class Action *StrategyReader::parseSlidingWindowFitAction(xmlNode *node)
{
	SlidingWindowFitAction *newAction = new SlidingWindowFitAction();
	newAction->Parameters().frequencyDirectionKernelSize = getDouble(node, "frequency-direction-kernel-size");
	newAction->Parameters().frequencyDirectionWindowSize = getInt(node, "frequency-direction-window-size");
	newAction->Parameters().method = (enum SlidingWindowFitParameters::Method) getInt(node, "method");
	newAction->Parameters().timeDirectionKernelSize = getDouble(node, "time-direction-kernel-size");
	newAction->Parameters().timeDirectionWindowSize = getInt(node, "time-direction-window-size");
	return newAction;
}

class Action *StrategyReader::parseStatisticalFlagAction(xmlNode *node)
{
	StatisticalFlagAction *newAction = new StatisticalFlagAction();
	newAction->SetEnlargeFrequencySize(getInt(node, "enlarge-frequency-size"));
	newAction->SetEnlargeTimeSize(getInt(node, "enlarge-time-size"));
	newAction->SetMaxContaminatedFrequenciesRatio(getDouble(node, "max-contaminated-frequencies-ratio"));
	newAction->SetMaxContaminatedTimesRatio(getDouble(node, "max-contaminated-times-ratio"));
	newAction->SetMinimumGoodFrequencyRatio(getDouble(node, "minimum-good-frequency-ratio"));
	newAction->SetMinimumGoodTimeRatio(getDouble(node, "minimum-good-time-ratio"));
	return newAction;
}

class Action *StrategyReader::parseSVDAction(xmlNode *node)
{
	SVDAction *newAction = new SVDAction();
	newAction->SetSingularValueCount(getInt(node, "singular-value-count"));
	return newAction;
}

class Action *StrategyReader::parseSumThresholdAction(xmlNode *node)
{
	SumThresholdAction *newAction = new SumThresholdAction();
	newAction->SetBaseSensitivity(getDouble(node, "base-sensitivity"));
	newAction->SetTimeDirectionFlagging(getBool(node, "time-direction-flagging"));
	newAction->SetFrequencyDirectionFlagging(getBool(node, "frequency-direction-flagging"));
	return newAction;
}

class Action *StrategyReader::parseTimeConvolutionAction(xmlNode *node)
{
	TimeConvolutionAction *newAction = new TimeConvolutionAction();
	newAction->SetOperation((enum TimeConvolutionAction::Operation) getInt(node, "operation"));
	newAction->SetSincScale(getDouble(node, "sinc-scale"));
	newAction->SetIsSincScaleInSamples(getBool(node, "is-sinc-scale-in-samples"));
	newAction->SetDirectionRad(getDouble(node, "direction-rad"));
	newAction->SetEtaParameter(getDouble(node, "eta-parameter"));
	newAction->SetAutoAngle(getBool(node, "auto-angle"));
	newAction->SetIterations(getInt(node, "iterations"));
	return newAction;
}

class Action *StrategyReader::parseTimeSelectionAction(xmlNode *node)
{
	TimeSelectionAction *newAction = new TimeSelectionAction();
	newAction->SetThreshold(getDouble(node, "threshold"));
	return newAction;
}

class Action *StrategyReader::parseUVProjectAction(xmlNode *node)
{
	UVProjectAction *newAction = new UVProjectAction();
	newAction->SetDirectionRad(getDouble(node, "direction-rad"));
	newAction->SetEtaParameter(getDouble(node, "eta-parameter"));
	newAction->SetDestResolutionFactor(getDouble(node, "dest-resolution-factor"));
	newAction->SetReverse(getBool(node, "reverse"));
	newAction->SetOnRevised(getBool(node, "on-revised"));
	newAction->SetOnContaminated(getBool(node, "on-contaminated"));
	return newAction;
}

class Action *StrategyReader::parseWriteDataAction(xmlNode *)
{
	WriteDataAction *newAction = new WriteDataAction();
	return newAction;
}

class Action *StrategyReader::parseWriteFlagsAction(xmlNode *)
{
	WriteFlagsAction *newAction = new WriteFlagsAction();
	return newAction;
}

} // end of namespace
