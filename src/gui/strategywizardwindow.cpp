#include "strategywizardwindow.h"
#include "interfaces.h"

#include <gtkmm/stock.h>

#include "../strategy/control/defaultstrategy.h"

StrategyWizardWindow::StrategyWizardWindow(class StrategyController &controller) : Window(),
	_strategyController(controller),
	_telescopeLabel("Telescope:"),
	_telescopeCombo(false),
	_finishButton(Gtk::Stock::OK),
	_nextButton(Gtk::Stock::MEDIA_NEXT),
	_previousButton(Gtk::Stock::MEDIA_PREVIOUS),
	_transientsButton("Transients"),
	_lowFreqRadioButton("Low frequency"), _normFreqRadioButton("Normal frequency"), _highFreqRadioButton("High frequency"),
	_smallBandwidthButton("Small bandwidth"), _normBandwidthButton("Normal bandwidth"), _largeBandwidthButton("Large bandwidth"),
	_robustConvergenceButton("Robust convergence"), _normConvergenceButton("Normal convergence"), _fastConvergenceButton("Fast convergence"),
	_offAxisSourcesButton("Off-axis sources"),
	_unsensitiveButton("Unsensitive"), _normalSensitivityButton("Normal sensitivity"), _sensitiveButton("Sensitive"),
	_guiFriendlyButton("GUI friendly"), _clearFlagsButton("Clear existing flags"), _autoCorrelationButton("Auto-correlation")
{
	_telescopeSubBox.pack_start(_telescopeLabel);
	_telescopeList = Gtk::ListStore::create(_telescopeListColumns);
	addTelescope("Generic", rfiStrategy::DefaultStrategy::DEFAULT_STRATEGY);
	addTelescope("LOFAR (Low-Frequency Array, Europe)", rfiStrategy::DefaultStrategy::LOFAR_STRATEGY);
	addTelescope("MWA (Murchison Widefield Array, Australia)", rfiStrategy::DefaultStrategy::MWA_STRATEGY);
	addTelescope("WSRT (Westerbork Synth. Rad. Telesc., Netherlands)", rfiStrategy::DefaultStrategy::WSRT_STRATEGY);
	_telescopeCombo.set_model(_telescopeList);
	_telescopeCombo.pack_start(_telescopeListColumns.name, false);
	_telescopeCombo.signal_changed().connect(sigc::mem_fun(*this, &StrategyWizardWindow::updateSensitivities));
	_telescopeSubBox.pack_start(_telescopeCombo, false, false);
	_telescopeBox.pack_start(_telescopeSubBox, false, false);
	_notebook.append_page(_telescopeBox, "Telescope");
	_notebook.signal_switch_page().connect(
		sigc::mem_fun(*this, &StrategyWizardWindow::onPageSwitched));
	
	initializeOptionPage();
	
	_mainBox.pack_start(_notebook, true, true);
	
	_previousButton.signal_clicked().connect(
		sigc::mem_fun(*this, &StrategyWizardWindow::onPreviousClicked));
	_buttonBox.pack_start(_previousButton, true, false);
	_nextButton.signal_clicked().connect(
		sigc::mem_fun(*this, &StrategyWizardWindow::onNextClicked));
	_buttonBox.pack_start(_nextButton, true, false);
	_finishButton.signal_clicked().connect(
		sigc::mem_fun(*this, &StrategyWizardWindow::onFinishClicked));
	_buttonBox.pack_end(_finishButton, true, false);
	_finishButton.set_sensitive(false);
	_mainBox.pack_end(_buttonBox, false, false);
	
	add(_mainBox);
	_mainBox.show_all();
}

void StrategyWizardWindow::initializeOptionPage()
{
	_optionsLeftBox.pack_start(_transientsButton, true, true);
	Gtk::RadioButton::Group freqGroup;
	_lowFreqRadioButton.set_group(freqGroup);
	_optionsLeftBox.pack_start(_lowFreqRadioButton, true, true);
	_normFreqRadioButton.set_group(freqGroup);
	_normFreqRadioButton.set_active(true);
	_optionsLeftBox.pack_start(_normFreqRadioButton, true, true);
	_highFreqRadioButton.set_group(freqGroup);
	_optionsLeftBox.pack_start(_highFreqRadioButton, true, true);
	
	Gtk::RadioButton::Group bandwidthGroup;
	_optionsRightBox.pack_start(_smallBandwidthButton, true, true);
	_smallBandwidthButton.set_group(bandwidthGroup);
	_optionsRightBox.pack_start(_normBandwidthButton, true, true);
	_normBandwidthButton.set_group(bandwidthGroup);
	_normBandwidthButton.set_active(true);
	_optionsRightBox.pack_start(_largeBandwidthButton, true, true);
	_largeBandwidthButton.set_group(bandwidthGroup);
	
	Gtk::RadioButton::Group convergenceGroup;
	_optionsLeftBox.pack_start(_robustConvergenceButton, true, true);
	_robustConvergenceButton.set_group(convergenceGroup);
	_optionsLeftBox.pack_start(_normConvergenceButton, true, true);
	_normConvergenceButton.set_group(convergenceGroup);
	_normConvergenceButton.set_active(true);
	_optionsLeftBox.pack_start(_fastConvergenceButton, true, true);
	_fastConvergenceButton.set_group(convergenceGroup);
	
	_optionsRightBox.pack_start(_offAxisSourcesButton, true, true);
	
	Gtk::RadioButton::Group sensitivityGroup;
	_optionsRightBox.pack_start(_unsensitiveButton, true, true);
	_unsensitiveButton.set_group(sensitivityGroup);
	_optionsRightBox.pack_start(_normalSensitivityButton, true, true);
	_normalSensitivityButton.set_group(sensitivityGroup);
	_normalSensitivityButton.set_active(true);
	_optionsRightBox.pack_start(_sensitiveButton, true, true);
	_sensitiveButton.set_group(sensitivityGroup);
	
	_optionsLeftBox.pack_start(_guiFriendlyButton, true, true);
	_guiFriendlyButton.set_active(true);
	_optionsLeftBox.pack_start(_clearFlagsButton, true, true);
	_optionsLeftBox.pack_start(_autoCorrelationButton, true, true);
	_optionsBox.pack_start(_optionsLeftBox);
	_optionsBox.pack_start(_optionsRightBox);
	_notebook.append_page(_optionsBox, "Options");
}

void StrategyWizardWindow::addTelescope(const Glib::ustring& name, int val)
{
	Gtk::TreeModel::iterator row = _telescopeList->append();
	(*row)[_telescopeListColumns.name] = name;
	(*row)[_telescopeListColumns.val] = val;
}

void StrategyWizardWindow::onPreviousClicked()
{
	_notebook.set_current_page(0);
}

void StrategyWizardWindow::onNextClicked()
{
	_notebook.set_current_page(1);
}

void StrategyWizardWindow::onFinishClicked()
{
	const enum rfiStrategy::DefaultStrategy::DefaultStrategyId telescopeId =
		(enum rfiStrategy::DefaultStrategy::DefaultStrategyId) (int) ((*_telescopeCombo.get_active())[_telescopeListColumns.val]);
		
	int flags = rfiStrategy::DefaultStrategy::FLAG_NONE;
	if(_lowFreqRadioButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_LOW_FREQUENCY;
	if(_highFreqRadioButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_HIGH_FREQUENCY;
	if(_largeBandwidthButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_LARGE_BANDWIDTH;
	if(_smallBandwidthButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_SMALL_BANDWIDTH;
	if(_transientsButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_TRANSIENTS;
	if(_robustConvergenceButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_ROBUST;
	if(_fastConvergenceButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_FAST;
	if(_offAxisSourcesButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_OFF_AXIS_SOURCES;
	if(_unsensitiveButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_UNSENSITIVE;
	if(_sensitiveButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_SENSITIVE;
	if(_guiFriendlyButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_GUI_FRIENDLY;
	if(_clearFlagsButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_CLEAR_FLAGS;
	if(_autoCorrelationButton.get_active())
		flags |= rfiStrategy::DefaultStrategy::FLAG_AUTO_CORRELATION;
	
	rfiStrategy::Strategy *strategy =
		rfiStrategy::DefaultStrategy::CreateStrategy(telescopeId, flags);
		
	_strategyController.SetStrategy(strategy);
	_strategyController.NotifyChange();
	
	hide();
}

void StrategyWizardWindow::onPageSwitched(GtkNotebookPage* page, guint pageNumber)
{
	updateSensitivities();
}

void StrategyWizardWindow::updateSensitivities()
{
	bool hasTelescope = (_telescopeCombo.get_active_row_number() != -1);
	int page = _notebook.get_current_page();
	_previousButton.set_sensitive(page!=0);
	_nextButton.set_sensitive(page!=1 && hasTelescope);
	_finishButton.set_sensitive(page==1 && hasTelescope);
}
