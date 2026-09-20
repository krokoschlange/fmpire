#ifndef MODULATOR_EDITOR_H_INCLUDED
#define MODULATOR_EDITOR_H_INCLUDED

#include "button.h"
#include "curve_editor.h"
#include "grid_container.h"
#include "mod_source_list.h"
#include "modulation_model.h"
#include "modulator_settings_panel.h"
#include "scroll_container.h"

namespace fmpire
{
class StateManager;

class ModulatorEditor :
	public GridContainer,
	public Button::Callback,
	public ModulationModel::Listener,
	public ModulatorSettingsPanel::Callback
{
public:
	ModulatorEditor(Widget* parent, StateManager& state_mgr);
	virtual ~ModulatorEditor() noexcept;

	virtual void on_press(Button* const button) override;
	virtual void on_release(Button* const button) override {}

	virtual void on_modulation_changed() override;

	virtual void on_grid_changed(const uint32_t x, const uint32_t y) override;

private:
	ModulationModel& model;

	Ref<GridContainer> list_column;
	Ref<ScrollContainer> list_scroll;
	Ref<ModSourceList> source_list;
	Ref<GridContainer> button_grid;
	Ref<Button> add_envelope_button;
	Ref<Button> add_lfo_button;
	Ref<Button> remove_button;

	Ref<CurveEditor> curve_editor;
	Ref<ModulatorSettingsPanel> settings_panel;
};

} // namespace fmpire

#endif // MODULATOR_EDITOR_H_INCLUDED
