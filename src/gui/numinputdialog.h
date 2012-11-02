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
#ifndef NUMINPUTDIALOG_H
#define NUMINPUTDIALOG_H

#include <cstdlib>
#include <sstream>

#include <gtkmm/box.h>
#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/stock.h>

class NumInputDialog : public Gtk::Dialog
{
	public:
		NumInputDialog(const Glib::ustring& title, const Glib::ustring& valueCaption, double defaultValue)
			: Dialog(title, true), _label(valueCaption)
		{
			_hBox.pack_start(_label);
			
			std::ostringstream s;
			s << defaultValue;
			_entry.set_text(s.str());
			_entry.set_activates_default(true);
			_hBox.pack_end(_entry);
			
			get_vbox()->pack_start(_hBox);
			_hBox.show_all();
			
			add_button(Gtk::Stock::OK, Gtk::RESPONSE_OK);
			add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
			set_default_response(Gtk::RESPONSE_OK);
		}
		
		double Value() const
		{
			return atof(_entry.get_text().c_str());
		}
	private:
		Gtk::Label _label;
		Gtk::Entry _entry;
		Gtk::HBox _hBox;
};

#endif // NUMINPUTDIALOG_H
