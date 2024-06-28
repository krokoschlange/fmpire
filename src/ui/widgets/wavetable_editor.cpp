#include "wavetable_editor.h"

#include "defines.h"
#include "int_editor.h"
#include "state_manager.h"
#include "waveform_editor.h"
#include "waveform_selector.h"

namespace fmpire
{

WavetableEditor::WavetableEditor(Widget* parent, StateManager& state_mgr) :
	GridContainer(parent),
	wavetable(nullptr),
	state_manager(state_mgr)
{
	state_manager.set_wavetable_editor(this);

	add_row(2, 0, 0, 50, 0);
	add_row(12, 0, 0, 100, 0);
	add_row(8, 0, 0, 50, 0);

	add_column(4, 0, 0, 50, 0);
	add_column(12, 0, 0, 100, 0);
	add_column(4, 0, 0, 50, 0);

	oscillator_selector = new IntEditor(this);
	oscillator_selector->set_limits(1, FMPIRE_OSC_COUNT);
	oscillator_selector->set_default_value(1);
	oscillator_selector->set_label("OSC");
	oscillator_selector->set_tooltip("Oscillator");
	put(oscillator_selector, 0, 0);

	waveform_editor = new WaveformEditor(this);
	put(waveform_editor, 0, 1, 2, 1);
	
	waveform_selector = new WaveformSelector(this);
	put(waveform_selector, 1, 2, 2, 1);
}

WavetableEditor::~WavetableEditor() noexcept
{
}

void WavetableEditor::select_oscillator(const size_t osc)
{
	oscillator_selector->set_value(osc + 1);
	wavetable = state_manager.get_wavetable(osc);
	repaint();
}

} // namespace fmpire
