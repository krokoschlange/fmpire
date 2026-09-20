#ifndef MODULATOR_SETTINGS_PANEL_H_INCLUDED
#define MODULATOR_SETTINGS_PANEL_H_INCLUDED

#include "border.h"
#include "grid_container.h"
#include "int_editor.h"
#include "knob.h"
#include "modulation_model.h"
#include "selector.h"

namespace fmpire
{

class ModulatorSettingsPanel :
	public GridContainer,
	public ModulationModel::Listener,
	public Selector::Callback,
	public IntEditor::Callback,
	public Knob::Callback
{
public:
	ModulatorSettingsPanel(Widget* parent, ModulationModel& modulation_model);
	virtual ~ModulatorSettingsPanel() noexcept;

	struct Callback
	{
		virtual void on_grid_changed(const uint32_t x, const uint32_t y) = 0;
	};

	void set_callback(Callback* const cb) { callback = cb; }

	virtual void on_modulation_changed() override;

	virtual void on_selected(Selector* const selector,
							 const int index,
							 const std::string& option) override;

	virtual void on_value_changed(IntEditor* const editor,
								  const int value) override;

	virtual void drag_started(Knob* const knob) override {}

	virtual void drag_ended(Knob* const knob) override {}

	virtual void value_changed(Knob* const knob, const float value) override;

private:
	ModulationModel& model;
	Callback* callback;

	size_t modulator_id;
	ModulatorSettings settings;
	bool refreshing;

	int beats_numerator;
	int beats_denominator;

	Ref<Border> shape_border;
	Ref<GridContainer> shape_grid;
	Ref<Selector> type_selector;
	Ref<IntEditor> sustain_editor;
	Ref<Knob> phase_knob;
	Ref<Knob> amount_knob;

	Ref<Border> timing_border;
	Ref<GridContainer> timing_grid;
	Ref<Selector> timing_selector;
	Ref<IntEditor> length_editor;
	Ref<GridContainer> beats_grid;
	Ref<IntEditor> numerator_editor;
	Ref<IntEditor> denominator_editor;

	Ref<Border> grid_border;
	Ref<GridContainer> grid_grid;
	Ref<IntEditor> grid_x_editor;
	Ref<IntEditor> grid_y_editor;

	void commit();
	void refresh();
};

} // namespace fmpire

#endif // MODULATOR_SETTINGS_PANEL_H_INCLUDED
