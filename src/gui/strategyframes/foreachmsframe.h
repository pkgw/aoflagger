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
#ifndef FOREACHMSFRAME_H
#define FOREACHMSFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "../../strategy/actions/foreachmsaction.h"

#include "../editstrategywindow.h"

class ForEachMSFrame : public Gtk::Frame {
	public:
		ForEachMSFrame(rfiStrategy::ForEachMSAction &action, EditStrategyWindow &editStrategyWindow)
		: Gtk::Frame("For each MS options"),
		_editStrategyWindow(editStrategyWindow), _action(action),
		_clearButton("Clear"),
		_addOneButton("Add one"),
		_addAllInDirButton("Add all in dir")
		{
			_addOneButton.signal_clicked().connect(sigc::mem_fun(*this, &ForEachMSFrame::onAddOneButtonClicked));
			_addAllInDirButton.signal_clicked().connect(sigc::mem_fun(*this, &ForEachMSFrame::onAddAllInDirButtonClicked));
			_clearButton.signal_clicked().connect(sigc::mem_fun(*this, &ForEachMSFrame::onClearButtonClicked));
			
			_fileListModel = Gtk::ListStore::create(_fileListColumns);
			fillFileList();
			
			_fileListView.set_model(_fileListModel);
			_fileListView.append_column("Title", _fileListColumns.title);
			_box.pack_start(_fileListView);
			_fileListView.show();
			
			_buttonBox.pack_start(_addOneButton);
			_addOneButton.show();
			
			_buttonBox.pack_start(_addAllInDirButton);
			_addAllInDirButton.show();
			
			_buttonBox.pack_start(_clearButton);
			_clearButton.show();
			
			_box.pack_end(_buttonBox, false, false, 0);
			_buttonBox.show();
			
			add(_box);
			_box.show();
		}
	private:
		EditStrategyWindow &_editStrategyWindow;
		rfiStrategy::ForEachMSAction &_action;
		Gtk::VBox _box;
		Gtk::HButtonBox _buttonBox;
		Gtk::Button _clearButton, _addOneButton, _addAllInDirButton;
		Gtk::TreeView _fileListView;
		Glib::RefPtr<Gtk::ListStore> _fileListModel;
		struct FileListModelColumns : public Gtk::TreeModelColumnRecord {
			public:
				FileListModelColumns() { add(title); }
				Gtk::TreeModelColumn<Glib::ustring> title;
		} _fileListColumns;
		
		void fillFileList()
		{
			_fileListModel->clear();
			std::vector<std::string> &filenames = _action.Filenames();
			for(std::vector<std::string>::const_iterator i=filenames.begin();i!=filenames.end();++i)
			{
				Gtk::TreeModel::iterator iter = _fileListModel->append();
				Gtk::TreeModel::Row row = *iter;
				row[_fileListColumns.title] = *i;
			}
		}
		
		void onAddOneButtonClicked()
		{
			Gtk::FileChooserDialog dialog("Select a measurement set",
							Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
			dialog.set_transient_for(_editStrategyWindow);
		
			//Add response buttons the the dialog:
			dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			dialog.add_button("Open", Gtk::RESPONSE_OK);
		
			int result = dialog.run();
		
			if(result == Gtk::RESPONSE_OK)
			{
				_action.Filenames().push_back(dialog.get_filename());
				fillFileList();
			}
		}
		
		void onAddAllInDirButtonClicked()
		{
			Gtk::FileChooserDialog dialog("Select a dir with measurement sets",
							Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
			dialog.set_transient_for(_editStrategyWindow);
		
			//Add response buttons the the dialog:
			dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			dialog.add_button("Open", Gtk::RESPONSE_OK);
		
			int result = dialog.run();
		
			if(result == Gtk::RESPONSE_OK)
			{
				_action.AddDirectory(dialog.get_filename());
				fillFileList();
			}
		}
		
		void onClearButtonClicked()
		{
			_action.Filenames().clear();
			fillFileList();
		}
};

#endif // FOREACHMSFRAME_H
