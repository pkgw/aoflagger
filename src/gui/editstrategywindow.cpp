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
#include <iostream>

#include <gtkmm/stock.h>
#include <gtkmm/messagedialog.h>

#include "../strategy/actions/iterationaction.h"
#include "../strategy/actions/strategyaction.h"

#include "../strategy/control/strategyreader.h"
#include "../strategy/control/strategywriter.h"
#include "../strategy/control/defaultstrategy.h"

#include "addstrategyactionmenu.h"
#include "editstrategywindow.h"
#include "interfaces.h"
#include "strategywizardwindow.h"

#include "strategyframes/absthresholdframe.h"
#include "strategyframes/baselineselectionframe.h"
#include "strategyframes/changeresolutionframe.h"
#include "strategyframes/cutareaframe.h"
#include "strategyframes/directionprofileframe.h"
#include "strategyframes/foreachbaselineframe.h"
#include "strategyframes/foreachmsframe.h"
#include "strategyframes/foreachpolarisationframe.h"
#include "strategyframes/foreachcomplexcomponentframe.h"
#include "strategyframes/frequencyconvolutionframe.h"
#include "strategyframes/frequencyselectionframe.h"
#include "strategyframes/fringestoppingframe.h"
#include "strategyframes/highpassfilterframe.h"
#include "strategyframes/iterationframe.h"
#include "strategyframes/plotframe.h"
#include "strategyframes/resamplingframe.h"
#include "strategyframes/setflaggingframe.h"
#include "strategyframes/setimageframe.h"
#include "strategyframes/slidingwindowfitframe.h"
#include "strategyframes/spatialcompositionframe.h"
#include "strategyframes/statisticalflaggingframe.h"
#include "strategyframes/svdframe.h"
#include "strategyframes/sumthresholdframe.h"
#include "strategyframes/timeconvolutionframe.h"
#include "strategyframes/timeselectionframe.h"
#include "strategyframes/uvprojectframe.h"

using namespace rfiStrategy;

EditStrategyWindow::EditStrategyWindow(StrategyController &strategyController)
 : Gtk::Window(), _strategyController(strategyController),
	_addActionButton(Gtk::Stock::ADD), _removeActionButton(Gtk::Stock::REMOVE),
	_moveUpButton(Gtk::Stock::GO_UP), _moveDownButton(Gtk::Stock::GO_DOWN),
	_addFOBButton("FOB"), _addFOMSButton("FOMS"),
	_loadEmptyButton(Gtk::Stock::NEW), _loadDefaultButton("Default"),
	_wizardButton("Wizard..."),
	_loadFullButton("Full"),
	_saveButton(Gtk::Stock::SAVE), _openButton(Gtk::Stock::OPEN),
	_disableUpdates(false),
	_rightFrame(0), _wizardWindow(0)
{
	_strategyController.SignalOnStrategyChanged().connect(sigc::mem_fun(*this, &EditStrategyWindow::onStrategyChanged));
	
	_store = Gtk::TreeStore::create(_columns);
	_view.set_model(_store);
	_view.append_column("Description", _columns.description);
	_viewScrollWindow.add(_view);
	_view.get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &EditStrategyWindow::onSelectionChanged));
	
	_viewScrollWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	_viewScrollWindow.set_size_request(100, 400);
	_strategyBox.pack_start(_viewScrollWindow);

	initEditButtons();

	initLoadDefaultsButtons();
	
	_paned.add1(_strategyBox);

	add(_paned);
	
	show_all();
	
	_strategy = &_strategyController.Strategy();
	fillStore();
}

EditStrategyWindow::~EditStrategyWindow()
{
	delete _addMenu;
}

void EditStrategyWindow::initEditButtons()
{
	_strategyEditButtonBox.pack_start(_addActionButton);
	_addActionButton.set_sensitive(false);
	_addMenu = new AddStrategyActionMenu(*this);
	_addActionButton.set_menu(*_addMenu);

	_strategyEditButtonBox.pack_start(_moveUpButton);
	_moveUpButton.set_sensitive(false);
	_moveUpButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onMoveUpClicked));

	_strategyEditButtonBox.pack_start(_moveDownButton);
	_moveDownButton.set_sensitive(false);
	_moveDownButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onMoveDownClicked));

	_strategyEditButtonBox.pack_start(_removeActionButton);
	_removeActionButton.set_sensitive(false);
	_removeActionButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onRemoveActionClicked));

	_strategyBox.pack_start(_strategyEditButtonBox, Gtk::PACK_SHRINK, 0);

	_strategyFileButtonBox.pack_start(_addFOBButton);
	_addFOBButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onAddFOBaseline));

	_strategyFileButtonBox.pack_start(_addFOMSButton);
	_addFOMSButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onAddFOMS));

	_strategyFileButtonBox.pack_start(_saveButton);
	_saveButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onSaveClicked));

	_strategyFileButtonBox.pack_start(_openButton);
	_openButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onOpenClicked));

	_strategyBox.pack_start(_strategyFileButtonBox, Gtk::PACK_SHRINK, 0);
}

void EditStrategyWindow::initLoadDefaultsButtons()
{
	_strategyLoadDefaultsButtonBox.pack_start(_loadEmptyButton);
	_loadEmptyButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onLoadEmptyClicked));

	_strategyLoadDefaultsButtonBox.pack_start(_loadDefaultButton);
	_loadDefaultButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onLoadDefaultClicked));
	
	_strategyLoadDefaultsButtonBox.pack_start(_wizardButton);
	_wizardButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onWizardClicked));

	_strategyLoadDefaultsButtonBox.pack_start(_loadFullButton);
	_loadFullButton.signal_clicked().connect(sigc::mem_fun(*this, &EditStrategyWindow::onLoadFullButtonClicked));

	_strategyBox.pack_start(_strategyLoadDefaultsButtonBox, Gtk::PACK_SHRINK, 0);
}

void EditStrategyWindow::onStrategyChanged()
{
	_strategy = &_strategyController.Strategy();
	clearRightFrame();
	_disableUpdates = true;
	_store->clear();
	fillStore();
	_disableUpdates = false;
	onSelectionChanged();
}

void EditStrategyWindow::fillStore()
{
	Gtk::TreeModel::iterator iter = _store->append();
	Gtk::TreeModel::Row row = *iter;
	row[_columns.action] = _strategy;
	row[_columns.description] = _strategy->Description();
	row[_columns.childIndex] = 0;
	for(size_t i = 0;i<_strategy->GetChildCount();++i)
	{
		fillStore(row, _strategy->GetChild(i), i);
	}
	_view.expand_all();
}

void EditStrategyWindow::fillStore(Gtk::TreeModel::Row &row, Action &action, size_t childIndex)
{
	Gtk::TreeModel::iterator iter = _store->append(row.children());
	Gtk::TreeModel::Row newRow = *iter;
	newRow[_columns.action] = &action;
	newRow[_columns.description] = action.Description();
	newRow[_columns.childIndex] = childIndex;
	ActionContainer *container = dynamic_cast<ActionContainer*>(&action);
	if(container != 0)
	{
		for(size_t i = 0;i<container->GetChildCount();++i)
		{
			fillStore(newRow, container->GetChild(i), i);
		}
	}
}

void EditStrategyWindow::onRemoveActionClicked()
{
	_disableUpdates = true;
	clearRightFrame();
	Action *action = GetSelectedAction();
	if(action != 0 && action->Parent() != 0)
	{
		action->Parent()->RemoveAndDelete(action);
		_store->clear();
		fillStore();
		_view.get_selection()->unselect_all();
	}
	_disableUpdates = false;
	onSelectionChanged();
}

void EditStrategyWindow::onMoveUpClicked()
{
	Action *action = GetSelectedAction();
	if(action != 0 && action->Parent() != 0)
	{
		ActionContainer *parent = static_cast<ActionContainer*>(action->Parent());
		size_t index = GetSelectedActionChildIndex();
		parent->MoveChildUp(index);
		_store->clear();
		fillStore();
		selectAction(action);
	}
}

void EditStrategyWindow::onMoveDownClicked()
{
	Action *action = GetSelectedAction();
	if(action != 0 && action->Parent() != 0)
	{
		ActionContainer *parent = static_cast<ActionContainer*>(action->Parent());
		size_t index = GetSelectedActionChildIndex();
		parent->MoveChildDown(index);
		_store->clear();
		fillStore();
		selectAction(action);
	}
}

void EditStrategyWindow::onSelectionChanged()
{
	if(!_disableUpdates)
	{
		Action *selectedAction = GetSelectedAction();
		if(selectedAction != 0)
		{
			clearRightFrame();
			
			_moveDownButton.set_sensitive(true);
			_moveUpButton.set_sensitive(true);
			_removeActionButton.set_sensitive(true);
			ActionContainer *container = dynamic_cast<rfiStrategy::ActionContainer*>(selectedAction);
			if(container != 0)
			{
				_addActionButton.set_sensitive(true);
			} else {
				_addActionButton.set_sensitive(false);
			}

			switch(selectedAction->Type())
			{
				case AbsThresholdActionType:
					showRight(new AbsThresholdFrame(*static_cast<rfiStrategy::AbsThresholdAction*>(selectedAction), *this));
					break;
				case BaselineSelectionActionType:
					showRight(new BaselineSelectionFrame(*static_cast<rfiStrategy::BaselineSelectionAction*>(selectedAction), *this));
					break;
				case ChangeResolutionActionType:
					showRight(new ChangeResolutionFrame(*static_cast<rfiStrategy::ChangeResolutionAction*>(selectedAction), *this));
					break;
				case CutAreaActionType:
					showRight(new CutAreaFrame(*static_cast<rfiStrategy::CutAreaAction*>(selectedAction), *this));
					break;
				case DirectionProfileActionType:
					showRight(new DirectionProfileFrame(*static_cast<rfiStrategy::DirectionProfileAction*>(selectedAction), *this));
					break;
				case FringeStopActionType:
					showRight(new FringeStoppingFrame(*static_cast<rfiStrategy::FringeStopAction*>(selectedAction), *this));
					break;
				case IterationBlockType:
					showRight(new IterationFrame(*static_cast<rfiStrategy::IterationBlock*>(selectedAction), *this));
					break;
				case SlidingWindowFitActionType:
					showRight(new SlidingWindowFitFrame(*static_cast<rfiStrategy::SlidingWindowFitAction*>(selectedAction), *this));
					break;
				case SVDActionType:
					showRight(new SVDFrame(*static_cast<rfiStrategy::SVDAction*>(selectedAction), *this));
					break;
				case ForEachBaselineActionType:
					showRight(new ForEachBaselineFrame(*static_cast<rfiStrategy::ForEachBaselineAction*>(selectedAction), *this));
					break;
				case ForEachComplexComponentActionType:
					showRight(new ForEachComplexComponentFrame(*static_cast<rfiStrategy::ForEachComplexComponentAction*>(selectedAction), *this));
					break;
				case ForEachMSActionType:
					showRight(new ForEachMSFrame(*static_cast<rfiStrategy::ForEachMSAction*>(selectedAction), *this));
					break;
				case ForEachPolarisationBlockType:
					showRight(new ForEachPolarisationFrame(*static_cast<rfiStrategy::ForEachPolarisationBlock*>(selectedAction), *this));
					break;
				case FrequencyConvolutionActionType:
					showRight(new FrequencyConvolutionFrame(*static_cast<rfiStrategy::FrequencyConvolutionAction*>(selectedAction), *this));
					break;
				case FrequencySelectionActionType:
					showRight(new FrequencySelectionFrame(*static_cast<rfiStrategy::FrequencySelectionAction*>(selectedAction), *this));
					break;
				case HighPassFilterActionType:
					showRight(new HighPassFilterFrame(*static_cast<rfiStrategy::HighPassFilterAction*>(selectedAction), *this));
					break;
				case PlotActionType:
					showRight(new StrategyPlotFrame(*static_cast<rfiStrategy::PlotAction*>(selectedAction), *this));
					break;
				case ResamplingActionType:
					showRight(new ResamplingFrame(*static_cast<rfiStrategy::ResamplingAction*>(selectedAction), *this));
					break;
				case SetImageActionType:
					showRight(new SetImageFrame(*static_cast<rfiStrategy::SetImageAction*>(selectedAction), *this));
					break;
				case SetFlaggingActionType:
					showRight(new SetFlaggingFrame(*static_cast<rfiStrategy::SetFlaggingAction*>(selectedAction), *this));
					break;
				case SpatialCompositionActionType:
					showRight(new SpatialCompositionFrame(*static_cast<rfiStrategy::SpatialCompositionAction*>(selectedAction), *this));
					break;
				case StatisticalFlagActionType:
					showRight(new StatisticalFlaggingFrame(*static_cast<rfiStrategy::StatisticalFlagAction*>(selectedAction), *this));
					break;
				case SumThresholdActionType:
					showRight(new SumThresholdFrame(*static_cast<rfiStrategy::SumThresholdAction*>(selectedAction), *this));
					break;
				case TimeConvolutionActionType:
					showRight(new TimeConvolutionFrame(*static_cast<rfiStrategy::TimeConvolutionAction*>(selectedAction), *this));
					break;
				case TimeSelectionActionType:
					showRight(new TimeSelectionFrame(*static_cast<rfiStrategy::TimeSelectionAction*>(selectedAction), *this));
					break;
				case UVProjectActionType:
					showRight(new UVProjectFrame(*static_cast<rfiStrategy::UVProjectAction*>(selectedAction), *this));
					break;
				default:
					break;
			}
		} else {
			_addActionButton.set_sensitive(false);
			_moveDownButton.set_sensitive(false);
			_moveUpButton.set_sensitive(false);
			_removeActionButton.set_sensitive(false);
		}
	}
}

void EditStrategyWindow::clearRightFrame()
{
	if(_rightFrame != 0)
	{
		delete _rightFrame;
		_rightFrame = 0;
	}
}

rfiStrategy::Action *EditStrategyWindow::GetSelectedAction()
{
	Gtk::TreeModel::iterator iter = _view.get_selection()->get_selected();
	if(iter) //If anything is selected
	{
		Gtk::TreeModel::Row row = *iter;
		return row[_columns.action];
	}
	else return 0;
}

size_t EditStrategyWindow::GetSelectedActionChildIndex()
{
	Gtk::TreeModel::iterator iter = _view.get_selection()->get_selected();
	if(iter) //If anything is selected
	{
		Gtk::TreeModel::Row row = *iter;
		return row[_columns.childIndex];
	}
	else return 0;
}

void EditStrategyWindow::AddAction(rfiStrategy::Action *newAction)
{
	Action *action = GetSelectedAction();
	if(action != 0)
	{
		rfiStrategy::ActionContainer *container = dynamic_cast<rfiStrategy::ActionContainer*>(action);
		if(container != 0)
		{
			container->Add(newAction);
			_store->clear();
			fillStore();
			_view.get_selection()->unselect_all();
			selectAction(newAction);
		}
	}
}

void EditStrategyWindow::selectAction(rfiStrategy::Action *action)
{
	_view.get_selection()->select(findActionRow(action));
}

void EditStrategyWindow::UpdateAction(Action *action)
{
	if(action != 0)
	{
		Gtk::TreeModel::Row row = findActionRow(action);
		row[_columns.description] = action->Description();
	}
}

Gtk::TreeModel::Row EditStrategyWindow::findActionRow(rfiStrategy::Action *action)
{
	std::deque<Gtk::TreeModel::Row> rows;
	Gtk::TreeNodeChildren children = _store->children();
	for(Gtk::TreeModel::const_iterator iter = children.begin();iter!=children.end();++iter)
	{
		const Gtk::TreeModel::Row &row = (*iter);
		rows.push_back(row);
	}
	while(!rows.empty())
	{
		Gtk::TreeModel::Row row = rows.front();
		rows.pop_front();
		if(row[_columns.action] == action)
		{
			return row;
		}
		Gtk::TreeNodeChildren rowChildren = row.children();
		for(Gtk::TreeModel::const_iterator iter = rowChildren.begin();iter != rowChildren.end();++iter)
		{
			Gtk::TreeModel::Row childRow = *iter;
			rows.push_back(childRow);
		}
	}
	throw BadUsageException("Could not find row in view");
}

void EditStrategyWindow::onLoadEmptyClicked()
{
	_strategy->RemoveAll();
	_store->clear();
	fillStore();
}

void EditStrategyWindow::onLoadDefaultClicked()
{
	_strategy->RemoveAll();
	DefaultStrategy::LoadStrategy(*_strategy, rfiStrategy::DefaultStrategy::GENERIC_TELESCOPE, rfiStrategy::DefaultStrategy::FLAG_GUI_FRIENDLY);
	_store->clear();
	fillStore();
}

void EditStrategyWindow::onLoadFullButtonClicked()
{
	_strategy->RemoveAll();
	DefaultStrategy::LoadFullStrategy(*_strategy, rfiStrategy::DefaultStrategy::GENERIC_TELESCOPE, rfiStrategy::DefaultStrategy::FLAG_GUI_FRIENDLY);
	_store->clear();
	fillStore();
}

void EditStrategyWindow::onSaveClicked()
{
  Gtk::FileChooserDialog dialog(*this, "Save strategy", Gtk::FILE_CHOOSER_ACTION_SAVE);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Save", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("RFI strategies");
  filter->add_pattern("*.rfis");
  filter->add_mime_type("text/rfistrategy+xml");
  dialog.add_filter(filter);

  int result = dialog.run();
  if(result == Gtk::RESPONSE_OK)
	{
		rfiStrategy::StrategyWriter writer;
		std::string filename(dialog.get_filename());
		if(filename.find('.') == std::string::npos)
			filename += ".rfis";
		writer.WriteToFile(*_strategy, filename);
		
	}
}

void EditStrategyWindow::onOpenClicked()
{
  Gtk::FileChooserDialog dialog(*this, "OPEN strategy", Gtk::FILE_CHOOSER_ACTION_OPEN);

  dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
  dialog.add_button("Open", Gtk::RESPONSE_OK);

  Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
  filter->set_name("RFI strategies");
  filter->add_pattern("*.rfis");
  filter->add_mime_type("text/rfistrategy+xml");
  dialog.add_filter(filter);

  int result = dialog.run();
  if(result == Gtk::RESPONSE_OK)
	{
		StrategyReader reader;
		std::string filename(dialog.get_filename());
		try {
			_strategy = reader.CreateStrategyFromFile(filename);
			_strategyController.SetStrategy(_strategy);
			_store->clear();
			fillStore();
		} catch(std::exception &e)
		{
			Gtk::MessageDialog dialog(*this, e.what(), false, Gtk::MESSAGE_ERROR);
			dialog.run();
		}
	}
}

void EditStrategyWindow::onAddFOBaseline()
{
	addContainerBetween(*_strategy, new rfiStrategy::ForEachBaselineAction());
	_store->clear();
	fillStore();
}

void EditStrategyWindow::onAddFOMS()
{
	addContainerBetween(*_strategy, new rfiStrategy::ForEachMSAction());
	_store->clear();
	fillStore();
}

void EditStrategyWindow::addContainerBetween(rfiStrategy::ActionContainer &root, rfiStrategy::ActionContainer *newContainer)
{
	while(root.GetChildCount() > 0)
	{
		Action *moveAction = &root.GetFirstChild();
		root.RemoveWithoutDelete(moveAction);
		newContainer->Add(moveAction);
	}
	root.Add(dynamic_cast<Action *>(newContainer));
}

void EditStrategyWindow::onWizardClicked()
{
	if(_wizardWindow == 0)
	{
		_wizardWindow = new StrategyWizardWindow(_strategyController);
		_wizardWindow->show();
	} else {
		_wizardWindow->show();
		_wizardWindow->raise();
	}
}
