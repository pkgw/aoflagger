#include "plotwindow.h"

#include "plot/plotmanager.h"

PlotWindow::PlotWindow(PlotManager &plotManager) :
	_plotManager(plotManager)
{
	plotManager.OnUpdate() = boost::bind(&PlotWindow::handleUpdate, this);
	
	_hBox.pack_start(_plotWidget, Gtk::PACK_EXPAND_WIDGET);
	
	_plotListStore = Gtk::ListStore::create(_plotListColumns);
	_plotListView.set_model(_plotListStore);
	_plotListView.append_column("Title", _plotListColumns._name);
	_hBox.pack_end(_plotListView, false, false, 3);
	
	add(_hBox);
	_hBox.show_all();
}

void PlotWindow::handleUpdate()
{
	updatePlotList();
	
	const std::vector<Plot2D*> &plots = _plotManager.Items();
	Plot2D &lastPlot = **plots.rbegin();
	_plotWidget.SetPlot(lastPlot);
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
