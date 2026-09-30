#include "modulator_editor.h"

#include "state_manager.h"

namespace fmpire
{

ModulatorEditor::ModulatorEditor(Widget* parent, StateManager& state_mgr) :
	GridContainer(parent),
	model(state_mgr.get_modulation())
{
	add_row(1, 0, 0, 0, 0);
	add_column(4, 0, 0, 0, 0);
	add_column(12, 0, 0, 0, 0);
	add_column(4, 0, 0, 0, 0);

	// left: source list with Add/Remove underneath
	list_column = new GridContainer(this);
	list_column->add_row(1, 0, 0, 0, 0);
	list_column->add_row(0.1f, 0, 0, 0, 0);
	list_column->add_column(1, 0, 0, 0, 0);
	put(list_column, 0, 0);

	list_scroll = new ScrollContainer(list_column);
	list_scroll->set_scroll_mode(ScrollContainer::VERTICAL);
	list_column->put(list_scroll, 0, 0);

	source_list = new ModSourceList(list_scroll, state_mgr);

	button_grid = new GridContainer(list_column);
	button_grid->add_row(1, 0, 0, 0, 0);
	button_grid->add_column(1, 0, 0, 0, 0);
	button_grid->add_column(1, 0, 0, 0, 0);
	button_grid->add_column(1, 0, 0, 0, 0);
	list_column->put(button_grid, 1, 0);

	add_envelope_button = new Button(button_grid);
	add_envelope_button->set_text("+ENV");
	add_envelope_button->set_callback(this);
	button_grid->put(add_envelope_button, 0, 0);

	add_lfo_button = new Button(button_grid);
	add_lfo_button->set_text("+LFO");
	add_lfo_button->set_callback(this);
	button_grid->put(add_lfo_button, 0, 1);

	remove_button = new Button(button_grid);
	remove_button->set_text("-");
	remove_button->set_callback(this);
	button_grid->put(remove_button, 0, 2);

	// middle: curve
	curve_editor = new CurveEditor(this, model);
	put(curve_editor, 0, 1);

	// right: settings
	settings_panel = new ModulatorSettingsPanel(this, model);
	settings_panel->set_callback(this);
	put(settings_panel, 0, 2);

	model.add_listener(this);
	on_modulation_changed();
}

ModulatorEditor::~ModulatorEditor() noexcept
{
	model.remove_listener(this);
}

void ModulatorEditor::on_press(Button* const button)
{
	if (button == add_envelope_button)
	{
		model.add_modulator(ModulatorType::ENVELOPE);
	}
	else if (button == add_lfo_button)
	{
		model.add_modulator(ModulatorType::LFO);
	}
	else if (button == remove_button)
	{
		model.remove_modulator(model.get_selected_modulator());
	}
}

void ModulatorEditor::on_modulation_changed()
{
	list_scroll->set_scroll_area(0, source_list->get_content_height());
	remove_button->set_enabled(
		model.has_modulator(model.get_selected_modulator()));
	repaint();
}

void ModulatorEditor::on_grid_changed(const uint32_t x, const uint32_t y)
{
	curve_editor->set_grid(x, y);
}

} // namespace fmpire
