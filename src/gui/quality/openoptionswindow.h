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
#ifndef GUI_QUALITY__OPEN_OPTIONS_WINDOW_H
#define GUI_QUALITY__OPEN_OPTIONS_WINDOW_H

#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/stock.h>
#include <gtkmm/window.h>

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class OpenOptionsWindow : public Gtk::Window {
	public:
		OpenOptionsWindow() :
			_downsampleTimeButton("Lower time resolution (faster plots)"),
			_downsampleFreqButton("Lower frequency resolution (faster plots)"),
			_correctHistograms("Correct histograms for frequence response"),
			_cancelButton(Gtk::Stock::CANCEL),
			_openButton(Gtk::Stock::OPEN)
		{
			_timeBox.pack_start(_downsampleTimeButton);
			_downsampleTimeButton.set_active(true);
			
			_timeBox.pack_start(_timeDownsampleEntry);
			_timeDownsampleEntry.set_text("1000");
			
			_box.pack_start(_timeBox);
			
			_freqBox.pack_start(_downsampleFreqButton);
			_downsampleFreqButton.set_active(true);
			
			_freqBox.pack_start(_freqDownsampleEntry);
			_freqDownsampleEntry.set_text("1000");
			
			_box.pack_start(_freqBox);
			
			_box.pack_start(_correctHistograms);
			
			_buttonBox.pack_start(_cancelButton);
			
			_buttonBox.pack_start(_openButton);
			_openButton.signal_clicked().connect(sigc::mem_fun(*this, &OpenOptionsWindow::onOpen));
			
			_box.pack_start(_buttonBox);
			
			add(_box);
			_box.show_all();
		}
		
    ~OpenOptionsWindow()
    {
		}
		
		void ShowForFile(const std::string &file)
		{
			_file = file;
			show();
		}
		
		sigc::signal<void, std::string, bool, bool, size_t, size_t, bool> &SignalOpen() { return _signalOpen; }
	private:
		void onOpen()
		{
			hide();
			size_t timeRes = atol(_timeDownsampleEntry.get_text().c_str());
			size_t freqRes = atol(_freqDownsampleEntry.get_text().c_str());
			_signalOpen.emit(_file, _downsampleTimeButton.get_active(), _downsampleFreqButton.get_active(), timeRes, freqRes,
			_correctHistograms.get_active()
			);
			_file.clear();
		}
		
		Gtk::VBox _box;
		Gtk::HBox _timeBox;
		Gtk::CheckButton _downsampleTimeButton;
		Gtk::Entry _timeDownsampleEntry;
		Gtk::HBox _freqBox;
		Gtk::CheckButton _downsampleFreqButton;
		Gtk::Entry _freqDownsampleEntry;
		Gtk::CheckButton _correctHistograms;
		Gtk::HButtonBox _buttonBox;
		Gtk::Button _cancelButton, _openButton;
		sigc::signal<void, std::string, bool, bool, size_t, size_t, bool> _signalOpen;
		
		std::string _file;
};

#endif
