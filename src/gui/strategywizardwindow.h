#ifndef STRATEGY_WIZARD_WINDOW_H
#define STRATEGY_WIZARD_WINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/notebook.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/combo.h>

class StrategyWizardWindow : public Gtk::Window
{
public:
	StrategyWizardWindow();
private:
	Gtk::Notebook _notebook;
	Gtk::VBox _mainBox, _telescopeBox, _optionsBox;
	Gtk::HBox _telescopeSubBox;
	Gtk::Label _telescopeLabel;
	Gtk::Combo _telescopeCombo;
	Gtk::HButtonBox _buttonBox;
	Gtk::Button _finishButton, _nextButton, _previousButton;
	Gtk::CheckButton _guiOptimizationButton;
	
	void onNextClicked();
	void onPreviousClicked();
	void onFinishClicked();
	void onPageSwitched(GtkNotebookPage *page, guint pageNumber);
	void updateSensitivities();
};

#endif
