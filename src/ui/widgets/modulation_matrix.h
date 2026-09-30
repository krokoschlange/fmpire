#ifndef MODULATION_MATRIX_H_INCLUDED
#define MODULATION_MATRIX_H_INCLUDED

#include "grid_container.h"
#include "knob.h"
#include "label.h"
#include "modulation_model.h"
#include "selector.h"

#include <array>
#include <vector>

namespace fmpire
{
class StateManager;

// The oscillator cross modulation matrix: pick AM, FM, PM or RM and set, for
// every pair of oscillators, how much the oscillator of the row (the
// modulator) modulates the oscillator of the column (the carrier). Every
// knob is a modulation target, so LFOs and envelopes can sweep the depths.
class ModulationMatrix :
	public GridContainer,
	public ModulationModel::Listener,
	public Selector::Callback,
	public Knob::Callback
{
public:
	ModulationMatrix(Widget* parent, StateManager& state_mgr);
	virtual ~ModulationMatrix() noexcept;

	virtual void on_modulation_changed() override;

	virtual void on_selected(Selector* const selector,
							 const int index,
							 const std::string& option) override;

	virtual void drag_started(Knob* const knob) override {}

	virtual void drag_ended(Knob* const knob) override {}

	virtual void value_changed(Knob* const knob, const float value) override;

private:
	ModulationModel& model;

	size_t type;

	Ref<Selector> type_selector;
	Ref<GridContainer> matrix_grid;
	std::vector<Ref<Label>> labels;
	// [modulator (row)][carrier (column)]
	std::array<Ref<Knob>, FMPIRE_OSC_COUNT * FMPIRE_OSC_COUNT> knobs;

	// True while the knobs are being set from the model.
	bool refreshing;

	void refresh();
};

} // namespace fmpire

#endif // MODULATION_MATRIX_H_INCLUDED
