#include "modulator_settings_panel.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace fmpire
{

namespace
{

constexpr float stacked_label_proportion = 0.4f;

void beats_to_fraction(const float beats, int& numerator, int& denominator)
{
	static const int denominators[] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 64};
	for (const int candidate : denominators)
	{
		const int candidate_numerator =
			static_cast<int>(std::lround(beats / 4.0f * candidate));
		if (candidate_numerator >= 1
			&& std::fabs(candidate_numerator * 4.0f / candidate - beats) < 1e-3f)
		{
			numerator = candidate_numerator;
			denominator = candidate;
			return;
		}
	}
	numerator = std::max(1, static_cast<int>(std::lround(beats)));
	denominator = 4;
}

} // namespace

ModulatorSettingsPanel::ModulatorSettingsPanel(
	Widget* parent,
	ModulationModel& modulation_model) :
	GridContainer(parent),
	model(modulation_model),
	callback(nullptr),
	modulator_id(ModulationModel::npos),
	refreshing(false),
	beats_numerator(1),
	beats_denominator(4)
{
	add_row(1, 0, 0, 120, 0);  // shape
	add_row(1, 0, 0, 84, 0);   // timing
	add_row(1, 0, 0, 58, 0);   // grid
	add_row(10, 0, 0, 0, 0);   // spacer
	add_column(1, 0, 0, 0, 0);

	// Shape
	shape_border = new Border(this);
	shape_grid = new GridContainer(shape_border);
	shape_grid->add_row(1, 0, 0, 26, 34);
	shape_grid->add_row(1, 0, 0, 26, 34);
	shape_grid->add_row(3, 0, 0, 48, 0);
	shape_grid->add_column(1, 0, 0, 0, 0);
	shape_grid->add_column(1, 0, 0, 0, 0);
	put(shape_border, 0, 0);

	type_selector = new Selector(shape_grid);
	type_selector->add_option("ENV");
	type_selector->add_option("LFO");
	type_selector->set_callback(this);
	shape_grid->put(type_selector, 0, 0, 1, 2);

	sustain_editor = new IntEditor(shape_grid);
	sustain_editor->set_label_position(IntEditor::LabelPosition::LEFT);
	sustain_editor->set_limits(0, 1);
	sustain_editor->set_label("Sustain pt");
	sustain_editor->set_callback(this);
	shape_grid->put(sustain_editor, 1, 0, 1, 2);

	phase_knob = new Knob(shape_grid);
	phase_knob->set_label("PHASE");
	phase_knob->set_tooltip("Phase Offset");
	phase_knob->set_callback(this);
	shape_grid->put(phase_knob, 2, 0);

	amount_knob = new Knob(shape_grid);
	amount_knob->set_label("AMT");
	amount_knob->set_tooltip("Amount");
	amount_knob->set_default_value(1.0f);
	amount_knob->set_callback(this);
	shape_grid->put(amount_knob, 2, 1);

	// Timing
	timing_border = new Border(this);
	timing_grid = new GridContainer(timing_border);
	timing_grid->add_row(1, 0, 0, 26, 34);
	timing_grid->add_row(1, 0, 0, 26, 34);
	timing_grid->add_row(1, 0, 0, 26, 34);
	timing_grid->add_column(1, 0, 0, 0, 0);
	put(timing_border, 1, 0);

	timing_selector = new Selector(timing_grid);
	timing_selector->add_option("SEC");
	timing_selector->add_option("BEATS");
	timing_selector->set_callback(this);
	timing_grid->put(timing_selector, 0, 0);

	length_editor = new IntEditor(timing_grid);
	length_editor->set_label_position(IntEditor::LabelPosition::LEFT);
	length_editor->set_limits(1, 600000);
	length_editor->set_default_value(1000);
	length_editor->set_label("Length ms");
	length_editor->set_callback(this);
	timing_grid->put(length_editor, 1, 0);

	beats_grid = new GridContainer(timing_grid);
	beats_grid->add_row(1, 0, 0, 0, 0);
	beats_grid->add_row(1, 0, 0, 0, 0);
	beats_grid->add_column(1, 0, 0, 0, 0);
	timing_grid->put(beats_grid, 1, 0, 2, 1);

	numerator_editor = new IntEditor(beats_grid);
	numerator_editor->set_label_position(IntEditor::LabelPosition::LEFT,
										 stacked_label_proportion);
	numerator_editor->set_limits(1, 64);
	numerator_editor->set_default_value(1);
	numerator_editor->set_label("Num");
	numerator_editor->set_callback(this);
	beats_grid->put(numerator_editor, 0, 0);

	denominator_editor = new IntEditor(beats_grid);
	denominator_editor->set_label_position(IntEditor::LabelPosition::LEFT,
										   stacked_label_proportion);
	denominator_editor->set_limits(1, 64);
	denominator_editor->set_default_value(4);
	denominator_editor->set_label("Den");
	denominator_editor->set_callback(this);
	beats_grid->put(denominator_editor, 1, 0);

	// Grid
	grid_border = new Border(this);
	grid_grid = new GridContainer(grid_border);
	grid_grid->add_row(1, 0, 0, 26, 34);
	grid_grid->add_row(1, 0, 0, 26, 34);
	grid_grid->add_column(1, 0, 0, 0, 0);
	put(grid_border, 2, 0);

	grid_x_editor = new IntEditor(grid_grid);
	grid_x_editor->set_label_position(IntEditor::LabelPosition::LEFT,
									  stacked_label_proportion);
	grid_x_editor->set_limits(0, std::numeric_limits<int>::max());
	grid_x_editor->set_label("Grid X");
	grid_x_editor->set_callback(this);
	grid_grid->put(grid_x_editor, 0, 0);

	grid_y_editor = new IntEditor(grid_grid);
	grid_y_editor->set_label_position(IntEditor::LabelPosition::LEFT,
									  stacked_label_proportion);
	grid_y_editor->set_limits(0, std::numeric_limits<int>::max());
	grid_y_editor->set_label("Grid Y");
	grid_y_editor->set_callback(this);
	grid_grid->put(grid_y_editor, 1, 0);

	model.add_listener(this);
	refresh();
}

ModulatorSettingsPanel::~ModulatorSettingsPanel() noexcept
{
	model.remove_listener(this);
}

void ModulatorSettingsPanel::on_modulation_changed()
{
	refresh();
}

void ModulatorSettingsPanel::refresh()
{
	modulator_id = model.get_selected_modulator();

	const bool has_modulator = model.has_modulator(modulator_id);
	shape_border->setVisible(has_modulator);
	timing_border->setVisible(has_modulator);
	if (!has_modulator)
	{
		repaint();
		return;
	}

	const ModulationModel::ModulatorEntry& entry = model.get_modulator(modulator_id);
	settings = entry.settings;

	refreshing = true;

	const bool is_envelope = settings.type == ModulatorType::ENVELOPE;
	type_selector->select(is_envelope ? 0 : 1, false);
	sustain_editor->setVisible(is_envelope);
	phase_knob->setVisible(!is_envelope);

	sustain_editor->set_limits(0, static_cast<int>(entry.curve.size()) - 1);
	sustain_editor->set_value(static_cast<int>(entry.curve.get_sustain_index()));
	phase_knob->set_value(settings.phase_offset, false);
	amount_knob->set_value(settings.amount, false);
	amount_knob->set_mod_target(model, TargetType::MOD_AMOUNT, modulator_id);

	timing_selector->select(settings.use_beats ? 1 : 0, false);
	length_editor->setVisible(!settings.use_beats);
	beats_grid->setVisible(settings.use_beats);
	length_editor->set_value(
		static_cast<int>(std::lround(settings.length_seconds * 1000.0f)));
	if (std::fabs(static_cast<float>(beats_numerator) / beats_denominator * 4.0f
				  - settings.length_beats)
		> 1e-3f)
	{
		beats_to_fraction(settings.length_beats,
						  beats_numerator,
						  beats_denominator);
	}
	numerator_editor->set_value(beats_numerator);
	denominator_editor->set_value(beats_denominator);

	refreshing = false;
	repaint();
}

void ModulatorSettingsPanel::commit()
{
	if (model.has_modulator(modulator_id))
	{
		model.set_modulator_settings(modulator_id, settings);
	}
}

void ModulatorSettingsPanel::on_selected(Selector* const selector,
										 const int index,
										 const std::string& option)
{
	if (refreshing || !model.has_modulator(modulator_id))
	{
		return;
	}

	if (selector == type_selector)
	{
		settings.type = index == 1 ? ModulatorType::LFO : ModulatorType::ENVELOPE;
		commit();
	}
	else if (selector == timing_selector)
	{
		settings.use_beats = index == 1;
		commit();
	}
}

void ModulatorSettingsPanel::on_value_changed(IntEditor* const editor,
											  const int value)
{
	if (editor == grid_x_editor || editor == grid_y_editor)
	{
		if (callback)
		{
			callback->on_grid_changed(grid_x_editor->get_value(),
									  grid_y_editor->get_value());
		}
		return;
	}

	if (refreshing || !model.has_modulator(modulator_id))
	{
		return;
	}

	if (editor == length_editor)
	{
		settings.length_seconds = value / 1000.0f;
		commit();
	}
	else if (editor == numerator_editor || editor == denominator_editor)
	{
		beats_numerator = numerator_editor->get_value();
		beats_denominator = denominator_editor->get_value();
		settings.length_beats =
			static_cast<float>(beats_numerator) / beats_denominator * 4.0f;
		commit();
	}
	else if (editor == sustain_editor)
	{
		Curve curve = model.get_modulator(modulator_id).curve;
		curve.set_sustain_index(static_cast<size_t>(std::max(value, 0)));
		model.set_modulator_curve(modulator_id, curve);
	}
}

void ModulatorSettingsPanel::value_changed(Knob* const knob, const float value)
{
	if (refreshing || !model.has_modulator(modulator_id))
	{
		return;
	}

	if (knob == phase_knob)
	{
		settings.phase_offset = value;
		commit();
	}
	else if (knob == amount_knob)
	{
		settings.amount = value;
		commit();
	}
}

} // namespace fmpire
