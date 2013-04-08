#ifndef STRATEGY_WIZARD_WINDOW_H
#define STRATEGY_WIZARD_WINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/combobox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/notebook.h>
#include <gtkmm/notebook.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/window.h>

class StrategyWizardWindow : public Gtk::Window
{
public:
	StrategyWizardWindow(class StrategyController &controller);
private:
	class StrategyController &_strategyController;
	
	Gtk::Notebook _notebook;
	Gtk::VBox _mainBox, _telescopeBox, _optionsLeftBox, _optionsRightBox;
	Gtk::HBox _telescopeSubBox, _optionsBox;
	Gtk::Label _telescopeLabel;
	Gtk::ComboBox _telescopeCombo;
	Glib::RefPtr<Gtk::ListStore> _telescopeList;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _finishButton, _nextButton, _previousButton;
	Gtk::CheckButton _transientsButton;
	Gtk::RadioButton _lowFreqRadioButton, _normFreqRadioButton, _highFreqRadioButton;
	Gtk::RadioButton _smallBandwidthButton, _normBandwidthButton, _largeBandwidthButton;
	Gtk::RadioButton _robustConvergenceButton, _normConvergenceButton, _fastConvergenceButton;
	Gtk::CheckButton _offAxisSourcesButton;
	Gtk::RadioButton _unsensitiveButton, _normalSensitivityButton, _sensitiveButton;
	Gtk::CheckButton _guiFriendlyButton, _clearFlagsButton, _autoCorrelationButton;
	
	void initializeOptionPage();
	void onNextClicked();
	void onPreviousClicked();
	void onFinishClicked();
	void onPageSwitched(Gtk::Widget *page, guint pageNumber);
	void updateSensitivities();
	void addTelescope(const Glib::ustring &name, int val);

	struct ModelColumns : public Gtk::TreeModelColumnRecord
	{
		ModelColumns() { add(name); add(val); }
		Gtk::TreeModelColumn<Glib::ustring> name;
		Gtk::TreeModelColumn<int> val;
	} _telescopeListColumns;
};

#endif
