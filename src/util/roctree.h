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
#ifndef ROCTREE_H
#define ROCTREE_H

#include <fstream>
#include <string>

class ROCTreeClassifier {
public:
	virtual void Perform(long double &falseRatio, long double &trueRatio, long double parameter) = 0;
	virtual ~ROCTreeClassifier() { }
};

/**
	@author A.R. Offringa <offringa@astro.rug.nl>
*/
class ROCTree {
	public:
		ROCTree(long double parameterRootValue, ROCTreeClassifier &classifier);
		~ROCTree();
		void Find(long double falseRatioValue, long double falseRatioPrecision, long double &actualFalseRatioValue, long double &trueRatioValue)
		{
			ROCTreeNode *node = Find(falseRatioValue, falseRatioPrecision);
			actualFalseRatioValue = node->falseRatio;
			trueRatioValue = node->trueRatio;
		}
		void Save(const std::string &filename); 
	private:
		struct ROCTreeNode {
			long double falseRatio, trueRatio, parameter;
			ROCTreeNode *left, *right;
		} *_tree;
		long double _parameterRootValue;
		ROCTreeClassifier &_classifier;

		ROCTreeNode *Find(long double falseRatioValue, long double falseRatioPrecision)
		{
			return Find(&_tree, _parameterRootValue, _parameterRootValue/2.0L, falseRatioValue, falseRatioPrecision, 0);
		}
		ROCTreeNode *Find(ROCTreeNode **node, long double parameter, long double stepSize, long double falseRatioValue, long double falseRatioPrecision, int depth);

		void SaveNode(std::ofstream &file, ROCTreeNode &node);
};

#endif
