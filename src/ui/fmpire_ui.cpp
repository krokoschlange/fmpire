#include "fmpire_ui.h"

#include "grid_container.h"
#include "keyboard_bar.h"
#include "modulation_matrix.h"
#include "oscillator_page.h"
#include "relative_container.h"
#include "source_list_page.h"
#include "tooltip.h"
#include "wavetable_editor.h"

namespace fmpire
{

FMpireUI::FMpireUI() :
	FMpireWindow(this),
	state_manager(this)
{
	grid = new GridContainer(this);
	grid->setAbsolutePos(0, 0);
	grid->setSize(100, 100);

	grid->add_row(1, 0, 3, 0, 100);
	grid->add_row(3, 0, 0, 0, 0);
	grid->add_row(1, 3, 0, 0, 90);
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

	oscillator_page = new OscillatorPage(grid, state_manager);
	grid->put(oscillator_page, 1, 0);

	mod_page = new SourceListPage(grid, state_manager, "");
	mod_page->set_body(new ModulationMatrix(mod_page, state_manager));
	grid->put(mod_page, 1, 0);

	fx_page = new SourceListPage(grid, state_manager, "Effects - coming soon");
	grid->put(fx_page, 1, 0);

	wavetable_editor = new WavetableEditor(grid, state_manager);
	grid->put(wavetable_editor, 1, 0);

	// not part of the tabs, so it stays visible on every page
	keyboard_bar = new KeyboardBar(grid);
	keyboard_bar->set_callback(this);
	grid->put(keyboard_bar, 2, 0);

	setSize(1024, 768);


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
	if (index < FMPIRE_MACRO_COUNT)
	{
		state_manager.on_macro_changed(index, value);
	}
	else if (index < FMPIRE_PLAYHEAD_BASE)
	{
		state_manager.get_modulation().set_route_meter(index
														   - FMPIRE_METER_BASE,
													   value);
	}
	else if (index < FMPIRE_PARAMETER_COUNT)
	{
		state_manager.get_modulation().set_playhead(index
														- FMPIRE_PLAYHEAD_BASE,
													value);
	}
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
	keyboard_bar->idle();
}

void FMpireUI::uiFileBrowserSelected(const char* filename)
{
	state_manager.on_file_browser_selected(filename);
}

void FMpireUI::onDisplay()
{
}

bool FMpireUI::onMouse(const MouseEvent& event)
{
	FMpireWidget* const previous_focus = get_focus();
	bool is_handled = UI::onMouse(event);

	if (event.press && previous_focus && get_focus() == previous_focus)
	{
		// the press didn't move focus elsewhere: drop it unless the press
		// landed on the focused widget itself
		if (!is_handled
			|| !previous_focus->getAbsoluteArea().contains(event.pos))
		{
			previous_focus->repaint();
			set_focus(nullptr);
		}
	}

	return is_handled;
}

bool FMpireUI::onMotion(const MotionEvent& event)
{
	get_tooltip().handle_motion(event);
	return UI::onMotion(event);
}

void FMpireUI::onResize(const ResizeEvent& ev)
{
	if (grid == nullptr)
	{
		return;
	}

	grid->setSize(ev.size);
}

void FMpireUI::switch_to_tab(const int tab)
{
	oscillator_page->hide();
	mod_page->hide();
	fx_page->hide();
	wavetable_editor->hide();
	switch (tab)
	{
	case 0:
		oscillator_page->show();
		break;
	case 1:
		mod_page->show();
		break;
	case 2:
		fx_page->show();
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

void FMpireUI::on_key_pressed(PianoKeyboard* const keyboard,
							  const int note,
							  const int velocity)
{
	sendNote(0, note, velocity);
}

void FMpireUI::on_key_released(PianoKeyboard* const keyboard, const int note)
{
	sendNote(0, note, 0);
}

} // namespace fmpire

START_NAMESPACE_DISTRHO

UI* createUI()
{
	return new fmpire::FMpireUI();
}

END_NAMESPACE_DISTRHO
