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
#ifndef EDITSTRATEGYWINDOW_H
#define EDITSTRATEGYWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/menutoolbutton.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/window.h>

#include "../strategy/control/types.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class EditStrategyWindow : public Gtk::Window
{
	public:
		EditStrategyWindow(class MSWindow &msWindow);
		~EditStrategyWindow();

		void AddAction(rfiStrategy::Action *newAction);
		void UpdateAction(rfiStrategy::Action *action);
	private:
		rfiStrategy::Action *GetSelectedAction();
		size_t GetSelectedActionChildIndex();
		void initEditButtons();
		void initLoadDefaultsButtons();
		void fillStore();
		void fillStore(Gtk::TreeModel::Row &row, rfiStrategy::Action &action, size_t childIndex);

		void onRemoveActionClicked();
		void onMoveUpClicked();
		void onMoveDownClicked();
		void onAddFOBaseline();
		void onAddFOMS();
		void onSelectionChanged();

		void onSaveClicked();
		void onOpenClicked();

		void onLoadEmptyClicked();
		void onLoadDefaultClicked();
		void onLoadOldClicked();
		void onLoad1ButtonClicked();
		void onLoad2ButtonClicked();
		void onLoad3ButtonClicked();

		void clearRightFrame();
		void selectAction(rfiStrategy::Action *action);
		void showRight(Gtk::Frame *newFrame)
		{
			clearRightFrame();
			_rightFrame = newFrame; 
			_paned.add2(*_rightFrame);
			_rightFrame->show();
		}
		Gtk::TreeModel::Row findActionRow(rfiStrategy::Action *action);
		void addContainerBetween(rfiStrategy::ActionContainer &root, rfiStrategy::ActionContainer *newContainer);

		class ModelColumns : public Gtk::TreeModelColumnRecord
		{
		public:
		
			ModelColumns()
				{ add(action); add(description); add(childIndex); }
		
			Gtk::TreeModelColumn<rfiStrategy::Action *> action;
			Gtk::TreeModelColumn<Glib::ustring> description;
			Gtk::TreeModelColumn<size_t> childIndex;
		};
		
		class MSWindow &_msWindow;

		Gtk::HPaned _paned;
		Gtk::VBox _strategyBox;
		rfiStrategy::Strategy *_strategy;
		Gtk::HButtonBox _strategyEditButtonBox, _strategyFileButtonBox, _strategyLoadDefaultsButtonBox;
		Gtk::MenuToolButton _addActionButton;
		Gtk::Button _removeActionButton, _moveUpButton, _moveDownButton;
		Gtk::Button _addFOBButton, _addFOMSButton;
		Gtk::Button _loadEmptyButton, _loadDefaultButton, _load1Button, _load2Button, _load3Button, _saveButton, _openButton;
		ModelColumns _columns;
		Gtk::ScrolledWindow _viewScrollWindow;
		Gtk::TreeView _view;
		Glib::RefPtr<Gtk::TreeStore> _store;
		Gtk::Menu *_addMenu;

		Gtk::Frame *_rightFrame;
};

#endif
