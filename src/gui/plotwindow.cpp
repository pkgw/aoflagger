#include "plotwindow.h"

#include "plot/plotmanager.h"

#include <gtkmm/stock.h>

PlotWindow::PlotWindow(PlotManager &plotManager) :
	_plotManager(plotManager),
	_clearButton(Gtk::Stock::CLEAR)
{
	Gtk::ToolButton();
	plotManager.OnUpdate() = boost::bind(&PlotWindow::handleUpdate, this);
	
	_hBox.pack_start(_plotWidget, Gtk::PACK_EXPAND_WIDGET);
	
	_plotListStore = Gtk::ListStore::create(_plotListColumns);
	_plotListView.set_model(_plotListStore);
	_plotListView.append_column("Plot title", _plotListColumns._name);
	_plotListView.get_selection()->signal_changed().connect(
		sigc::mem_fun(*this, &PlotWindow::onSelectedPlotChange));
	
	_clearButton.set_tooltip_text("Clear plots");
	_clearButton.signal_clicked().connect(
		sigc::mem_fun(*this, &PlotWindow::onClearPlotsPressed));
	_toolbar.set_icon_size(Gtk::ICON_SIZE_SMALL_TOOLBAR);
	_toolbar.set_toolbar_style(Gtk::TOOLBAR_ICONS);
	_toolbar.append(_clearButton);
	_sideBox.pack_start(_toolbar, Gtk::PACK_SHRINK);
	_sideBox.pack_start(_plotListView);
	
	_hBox.pack_end(_sideBox, false, false, 3);
	
	add(_hBox);
	_hBox.show_all();
}

void PlotWindow::handleUpdate()
{
	updatePlotList();
	
	const std::vector<Plot2D*> &plots = _plotManager.Items();
	if(!plots.empty())
	{
		Plot2D &lastPlot = **plots.rbegin();
		size_t index = plots.size()-1;
		_plotWidget.SetPlot(lastPlot);
		for(Gtk::TreeNodeChildren::iterator i = _plotListStore->children().begin();
				i!=_plotListStore->children().end(); ++i)
		{
			if((*i)[_plotListColumns._index] == index)
			{
				_plotListView.get_selection()->select(i);
				break;
			}
		}
	}
	show();
	raise();
}

void PlotWindow::updatePlotList()
{
	const std::vector<Plot2D*> &plots = _plotManager.Items();
	
	_plotListStore->clear();
	for(size_t index=0; index!=plots.size(); ++index)
	{
		const Plot2D &plot = *plots[index];
		Gtk::TreeModel::Row row = *_plotListStore->append();
		row[_plotListColumns._index] = index;
		row[_plotListColumns._name] = plot.Title();
	}
}

void PlotWindow::onSelectedPlotChange()
{
	Gtk::TreeModel::iterator iter = _plotListView.get_selection()->get_selected();
	if(iter) //If anything is selected
	{
		Gtk::TreeModel::Row row = *iter;
		size_t index = row[_plotListColumns._index];
		Plot2D &plot = *_plotManager.Items()[index];
		_plotWidget.SetPlot(plot);
	}
}

void PlotWindow::onClearPlotsPressed()
{
	_plotManager.Clear();
}
