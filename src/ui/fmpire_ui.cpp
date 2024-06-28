#include "fmpire_ui.h"

#include "grid_container.h"
#include "oscillator_bar.h"
#include "relative_container.h"
#include "tooltip.h"
#include "wavetable_editor.h"

namespace fmpire
{

FMpireUI::FMpireUI() :
	FMpireWindow(this),
	state_manager(this)
{
	setSize(1024, 768);
	grid = new GridContainer(this);
	grid->setAbsolutePos(0, 0);
	grid->setSize(100, 100);

	grid->add_row(1, 0, 3, 0, 100);
	grid->add_row(3, 0, 0, 0, 0);
	grid->add_column(1, 0, 0, 0, 0);

	top_bar = new RelativeContainer(grid);
	grid->put(top_bar, 0, 0);

	tab_selector = new Selector(top_bar);
	tab_selector->add_option("OSC");
	tab_selector->add_option("MOD");
	tab_selector->add_option("FX");
	tab_selector->add_option("WT");
	tab_selector->set_callback(this);
	top_bar->put(tab_selector, 0.25, 0, 0.5, 1);

	oscillator_bar = new OscillatorBar(grid, state_manager);
	grid->put(oscillator_bar, 1, 0);

	wavetable_editor = new WavetableEditor(grid, state_manager);
	grid->put(wavetable_editor, 1, 0);


	reinit_tooltip();
	get_tooltip().hide();

	switch_to_tab(0);
	repaint();
}

FMpireUI::~FMpireUI() noexcept
{
}

void FMpireUI::parameterChanged(uint32_t index, float value)
{
}

void FMpireUI::stateChanged(const char* key, const char* value)
{
	d_stdout("state change %s %s", key, value);
	std::string_view str_view(value);
	state_manager.state_changed(key, str_view);
}

void FMpireUI::uiIdle()
{
	get_tooltip().idle();
}

void FMpireUI::onDisplay()
{
}

bool FMpireUI::onMotion(const MotionEvent& event)
{
	get_tooltip().handle_motion(event);
	UI::onMotion(event);
	return false;
}

void FMpireUI::onResize(const ResizeEvent& ev)
{
	grid->setSize(ev.size);
}

void FMpireUI::switch_to_tab(const int tab)
{
	oscillator_bar->hide();
	wavetable_editor->hide();
	switch (tab)
	{
	case 0:
		oscillator_bar->show();
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		wavetable_editor->show();
		break;
	default:
		break;
	}
	tab_selector->select(tab);
	repaint();
}

void FMpireUI::on_selected(Selector* const selector,
						   const int index,
						   const std::string& option)
{
	switch_to_tab(index);
}

} // namespace fmpire


START_NAMESPACE_DISTRHO

UI* createUI()
{
	return new fmpire::FMpireUI();
}

END_NAMESPACE_DISTRHO
