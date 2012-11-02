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
#ifndef PROGRESSLISTENER_H
#define PROGRESSLISTENER_H

#include <string>
#include <vector>

#include "../strategy/control/types.h"

class ProgressListener
{
	private:
		std::vector<size_t> _totals, _progresses, _weights;
		double _taskProgress;
	protected:
		double TotalProgress()
		{
			double part = 1.0, total = 0.0;
			for(size_t i=0;i<_totals.size();++i)
			{
				total += part * (double) _weights[i] * (double) _progresses[i] / (double) _totals[i];
				
				part *= (double) _weights[i] / (double) _totals[i];
			}
			total += part * _taskProgress;
			if(total < 0.0)
				return 0.0;
			else if(total > 1.0)
				return 1.0;
			else
				return total;
		}
		size_t Depth() const { return _totals.size(); }
	public:
		ProgressListener() { }
		virtual ~ProgressListener() { }

		inline virtual void OnStartTask(const rfiStrategy::Action &action, size_t taskNo, size_t taskCount, const std::string &/*description*/, size_t weight = 1);
		
		/**
		 * Signifies the end of the current task. It's not allowed to call OnProgress() after a call
		 * to OnEndTask() until a new task has been started with OnStartTask().
		 */
		inline virtual void OnEndTask(const rfiStrategy::Action &action);

		inline virtual void OnProgress(const rfiStrategy::Action &action, size_t progress, size_t maxProgress);

		virtual void OnException(const rfiStrategy::Action &action, std::exception &thrownException) = 0;
};

class DummyProgressListener : public ProgressListener {
  virtual void OnStartTask(const rfiStrategy::Action &, size_t, size_t, const std::string &, size_t = 1)
    {}
  virtual void OnEndTask(const rfiStrategy::Action &)
    {}
  virtual void OnProgress(const rfiStrategy::Action &, size_t, size_t)
    {}
  virtual void OnException(const rfiStrategy::Action &, std::exception &)
    {}
};

#include "../strategy/actions/action.h"

void ProgressListener::OnStartTask(const rfiStrategy::Action &, size_t taskNo, size_t taskCount, const std::string &/*description*/, size_t weight)
{
	_totals.push_back(taskCount);
	_progresses.push_back(taskNo);
	_weights.push_back(weight);
	_taskProgress = 0.0;
}

void ProgressListener::OnEndTask(const rfiStrategy::Action &)
{
	_totals.pop_back();
	_progresses.pop_back();
	_weights.pop_back();
	_taskProgress = 1.0;
}

void ProgressListener::OnProgress(const rfiStrategy::Action &, size_t progress, size_t maxProgress)
{
	_taskProgress = (double) progress / maxProgress;
}

#endif // PROGRESSLISTENER_H
