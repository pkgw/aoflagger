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
#include "gotowindow.h"

#include <sstream>

#include <gtkmm/treemodel.h>

#include "../strategy/imagesets/msimageset.h"

#include "rfiguiwindow.h"


GoToWindow::GoToWindow(RFIGuiWindow &rfiGuiWindow) : Gtk::Window(),
	_antenna1Frame("Antenna 1"), _antenna2Frame("Antenna 2"),
	_bandFrame("Band"), _sequenceFrame("Time sequence"),
	_loadButton("Load"),
	_rfiGuiWindow(rfiGuiWindow),
	_imageSet(&dynamic_cast<rfiStrategy::MSImageSet&>(rfiGuiWindow.GetImageSet()))
{
	_antennaeStore = Gtk::ListStore::create(_antennaModelColumns);
	_bandStore = Gtk::ListStore::create(_bandModelColumns);
	_sequenceStore = Gtk::ListStore::create(_sequenceModelColumns);

	const std::vector<MeasurementSet::Sequence> &_sequences =
		_imageSet->Reader()->Set().GetSequences();

	const rfiStrategy::MSImageSetIndex &setIndex =
		static_cast<rfiStrategy::MSImageSetIndex&>(_rfiGuiWindow.GetImageSetIndex());
	const unsigned antenna1Index = _imageSet->GetAntenna1(setIndex);
	const unsigned antenna2Index = _imageSet->GetAntenna2(setIndex);
	const unsigned bandIndex = _imageSet->GetBand(setIndex);
	const unsigned sequenceIndex = _imageSet->GetSequenceId(setIndex);

	// First, the baseline pairs are iterated to get all antenna indices.
	std::set<size_t> set;
	for(std::vector<MeasurementSet::Sequence>::const_iterator i=_sequences.begin();
		i != _sequences.end() ; ++i)
	{
		set.insert(i->antenna1);
		set.insert(i->antenna2);
	}

	Gtk::TreeModel::iterator a1Row, a2Row, bandRow, sequenceRow;

	// Now we make a store that contains all antennas. This store is shared for both a1 and a2 views.
	for(std::set<size_t>::const_iterator i=set.begin();i!=set.end();++i)
	{
		Gtk::TreeModel::iterator iter = _antennaeStore->append();
		(*iter)[_antennaModelColumns.antennaIndex] = *i;
		AntennaInfo antenna = _imageSet->GetAntennaInfo(*i);
		(*iter)[_antennaModelColumns.antennaName] = antenna.name;
		if(antenna1Index == *i)
			a1Row = iter;
		if(antenna2Index == *i)
			a2Row = iter;
	}

	size_t bandCount = _imageSet->BandCount();
	for(size_t i=0;i<bandCount;++i)
	{
		Gtk::TreeModel::iterator iter = _bandStore->append();
		(*iter)[_bandModelColumns.bandIndex] = i;
		std::stringstream desc;
		BandInfo band = _imageSet->GetBandInfo(i);
		desc << Frequency::ToString(band.channels.front().frequencyHz);
		desc << " - ";
		desc << Frequency::ToString(band.channels.back().frequencyHz);
		(*iter)[_bandModelColumns.bandDescription] = desc.str();
		if(i == bandIndex)
			bandRow = iter;
	}
	
	size_t sequenceCount = _imageSet->SequenceCount();
	for(size_t i=0;i<sequenceCount;++i)
	{
		Gtk::TreeModel::iterator iter = _sequenceStore->append();
		(*iter)[_sequenceModelColumns.sequenceIndex] = i;
		std::stringstream desc;
		std::auto_ptr<rfiStrategy::MSImageSetIndex> index(_imageSet->Index(antenna1Index, antenna2Index, bandIndex, i));
		size_t fIndex = _imageSet->GetField(*index);
		FieldInfo field = _imageSet->GetFieldInfo(fIndex);
		desc << field.name << " (" << fIndex << ')';
		(*iter)[_sequenceModelColumns.sequenceDescription] = desc.str();
		if(i == sequenceIndex)
			sequenceRow = iter;
	}

	_antenna1View.set_model(_antennaeStore);
	_antenna1View.append_column("Index", _antennaModelColumns.antennaIndex);
	_antenna1View.append_column("Name", _antennaModelColumns.antennaName);
	_antenna1View.set_size_request(150, 512);
	_antenna1View.get_selection()->select(a1Row);
	_antenna1Scroll.add(_antenna1View);
	_antenna1Frame.add(_antenna1Scroll);
	_antenna1Scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_hBox.pack_start(_antenna1Frame);

	_antenna2View.set_model(_antennaeStore);
	_antenna2View.append_column("Index", _antennaModelColumns.antennaIndex);
	_antenna2View.append_column("Name", _antennaModelColumns.antennaName);
	_antenna2View.set_size_request(150, 512);
	_antenna2View.get_selection()->select(a2Row);
	_antenna2Scroll.add(_antenna2View);
	_antenna2Frame.add(_antenna2Scroll);
	_antenna2Scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_hBox.pack_start(_antenna2Frame);

	_bandView.set_model(_bandStore);
	_bandView.append_column("Index", _bandModelColumns.bandIndex);
	_bandView.append_column("Description", _bandModelColumns.bandDescription);
	//_bandView.set_size_request(-1, 512);
	_bandView.get_selection()->select(bandRow);
	_bandScroll.add(_bandView);
	_bandFrame.add(_bandScroll);
	_bandScroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_bandFrameBox.pack_start(_bandFrame);

	_sequenceView.set_model(_sequenceStore);
	_sequenceView.append_column("Index", _sequenceModelColumns.sequenceIndex);
	_sequenceView.append_column("Source", _sequenceModelColumns.sequenceDescription);
	//_sequenceView.set_size_request(-1, 512);
	_sequenceView.get_selection()->select(sequenceRow);
	_sequenceScroll.add(_sequenceView);
	_sequenceFrame.add(_sequenceScroll);
	_sequenceScroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
	_bandFrameBox.pack_start(_sequenceFrame);
	
	_hBox.pack_start(_bandFrameBox);

	_vBox.pack_start(_hBox);

	_loadButton.signal_clicked().connect(sigc::mem_fun(*this, &GoToWindow::onLoadClicked));
	_buttonBox.pack_start(_loadButton);

	_vBox.pack_start(_buttonBox, Gtk::PACK_SHRINK, 0);

	add(_vBox);
	_vBox.show_all();
}

GoToWindow::~GoToWindow()
{
}

void GoToWindow::onLoadClicked()
{
	Glib::RefPtr<Gtk::TreeSelection> a1 =
    _antenna1View.get_selection();
	Gtk::TreeModel::iterator iterA1 = a1->get_selected();

	Glib::RefPtr<Gtk::TreeSelection> a2 =
    _antenna2View.get_selection();
	Gtk::TreeModel::iterator iterA2 = a2->get_selected();

	Glib::RefPtr<Gtk::TreeSelection> b =
    _bandView.get_selection();
	Gtk::TreeModel::iterator iterB = b->get_selected();

	Glib::RefPtr<Gtk::TreeSelection> s =
    _sequenceView.get_selection();
	Gtk::TreeModel::iterator iterS = s->get_selected();
	
	if(iterA1 && iterA2 && iterB && iterS)
	{
		Gtk::TreeModel::Row a1Row = *iterA1;
		Gtk::TreeModel::Row a2Row = *iterA2;
		Gtk::TreeModel::Row bRow = *iterB;
		Gtk::TreeModel::Row sRow = *iterS;
		size_t a1Index = a1Row[_antennaModelColumns.antennaIndex];
		size_t a2Index = a2Row[_antennaModelColumns.antennaIndex];
		size_t bIndex = bRow[_bandModelColumns.bandIndex];
		size_t sIndex = sRow[_sequenceModelColumns.sequenceIndex];
		_rfiGuiWindow.SetImageSetIndex(_imageSet->Index(a1Index, a2Index, bIndex, sIndex));
		hide();
	}
}

