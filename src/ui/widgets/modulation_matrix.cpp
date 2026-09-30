#include "modulation_matrix.h"

#include "state_manager.h"

#include <string>

namespace fmpire
{

namespace
{
const char* const type_names[matrix_type_count] = {"AM", "FM", "PM", "RM"};
} // namespace

ModulationMatrix::ModulationMatrix(Widget* parent, StateManager& state_mgr) :
	GridContainer(parent),
	model(state_mgr.get_modulation()),
	type(0),
	refreshing(false)
{
	add_row(1, 0, 0, 0, 34);
	add_row(12, 0, 0, 0, 0);
	add_column(1, 0, 0, 0, 0);

	type_selector = new Selector(this);
	for (const char* const name : type_names)
	{
		type_selector->add_option(name);
	}
	type_selector->select(static_cast<int>(type));
	type_selector->set_callback(this);
	put(type_selector, 0, 0);

	// a header row and column with the oscillator numbers, then the knobs
	matrix_grid = new GridContainer(this);
	matrix_grid->add_row(0.7f, 0, 0, 0, 0);
	matrix_grid->add_column(0.7f, 0, 0, 0, 0);
	for (size_t osc = 0; osc < FMPIRE_OSC_COUNT; osc++)
	{
		matrix_grid->add_row(1, 0, 0, 0, 0);
		matrix_grid->add_column(1, 0, 0, 0, 0);
	}
	put(matrix_grid, 1, 0);

	Ref<Label> corner = new Label(matrix_grid);
	corner->set_text("M\\C");
	matrix_grid->put(corner, 0, 0);
	labels.push_back(corner);

	for (size_t osc = 0; osc < FMPIRE_OSC_COUNT; osc++)
	{
		Ref<Label> carrier_label = new Label(matrix_grid);
		carrier_label->set_text("C" + std::to_string(osc + 1));
		matrix_grid->put(carrier_label, 0, osc + 1);
		labels.push_back(carrier_label);

		Ref<Label> modulator_label = new Label(matrix_grid);
		modulator_label->set_text("M" + std::to_string(osc + 1));
		matrix_grid->put(modulator_label, osc + 1, 0);
		labels.push_back(modulator_label);
	}

	for (size_t modulator = 0; modulator < FMPIRE_OSC_COUNT; modulator++)
	{
		for (size_t carrier = 0; carrier < FMPIRE_OSC_COUNT; carrier++)
		{
			Ref<Knob> knob = new Knob(matrix_grid);
			knob->set_default_value(0.0f);
			knob->set_value(0.0f);
			knob->set_callback(this);
			matrix_grid->put(knob, modulator + 1, carrier + 1);
			knobs[modulator * FMPIRE_OSC_COUNT + carrier] = knob;
		}
	}

	model.add_listener(this);
	refresh();
}

ModulationMatrix::~ModulationMatrix() noexcept
{
	model.remove_listener(this);
}

void ModulationMatrix::on_modulation_changed()
{
	refresh();
}

void ModulationMatrix::on_selected(Selector* const selector,
								   const int index,
								   const std::string& option)
{
	if (index < 0 || static_cast<size_t>(index) >= matrix_type_count)
	{
		return;
	}
	type = static_cast<size_t>(index);
	refresh();
}

void ModulationMatrix::value_changed(Knob* const knob, const float value)
{
	if (refreshing)
	{
		return;
	}

	for (size_t index = 0; index < knobs.size(); index++)
	{
		if (static_cast<Knob*>(knobs[index]) == knob)
		{
			model.set_matrix_depth(type,
								   index / FMPIRE_OSC_COUNT,
								   index % FMPIRE_OSC_COUNT,
								   value);
			return;
		}
	}
}

void ModulationMatrix::refresh()
{
	refreshing = true;

	for (size_t modulator = 0; modulator < FMPIRE_OSC_COUNT; modulator++)
	{
		for (size_t carrier = 0; carrier < FMPIRE_OSC_COUNT; carrier++)
		{
			Knob* const knob = knobs[modulator * FMPIRE_OSC_COUNT + carrier];
			knob->set_value(model.get_matrix_depth(type, modulator, carrier));
			knob->set_tooltip(std::string(type_names[type]) + " depth: OSC "
							  + std::to_string(modulator + 1)
							  + " modulates OSC "
							  + std::to_string(carrier + 1));
			knob->set_mod_target(model,
								 matrix_target(type),
								 matrix_target_object(carrier, modulator));
		}
	}

	refreshing = false;
	repaint();
}

} // namespace fmpire
