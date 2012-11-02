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
#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <glibmm.h>

#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>
#include <gtkmm/window.h>

#include "../types.h"

#include "../util/progresslistener.h"

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ProgressWindow : public Gtk::Window, public ProgressListener {
	public:
		ProgressWindow(class MSWindow &parentWindow);
		~ProgressWindow();

		virtual void OnStartTask(const rfiStrategy::Action &action, size_t taskNo, size_t taskCount, const std::string &description, size_t weight=1);
		virtual void OnEndTask(const rfiStrategy::Action &action);
		virtual void OnProgress(const rfiStrategy::Action &action, size_t progress, size_t maxProgress);
		virtual void OnException(const rfiStrategy::Action &action, std::exception &thrownException);
	private:
		void UpdateProgress();
		Glib::Dispatcher _progressChangeSignal;
		boost::mutex _mutex;

		Gtk::Label
			_currentTaskTitleLabel, _currentTaskLabel,
			_timeElapsedTitleLabel, _timeElapsedLabel,
			_timeEstimatedTitleLabel, _timeEstimatedLabel;
		
		Gtk::VBox _topBox;
		Gtk::ProgressBar _progressBar;

		double _progressFraction;
		std::vector<std::string> _tasks;
		struct Ratio {
			Ratio(size_t i, size_t c) : index(i), count(c) { }
			size_t index, count;
			};
		std::vector<Ratio> _ratios;
		boost::posix_time::ptime _startTime;
		bool _started;
		bool _exceptionOccured;
		std::string _exceptionType;
		std::string _exceptionDescription;

		class MSWindow &_parentWindow;
		double _progress;
};

#endif
