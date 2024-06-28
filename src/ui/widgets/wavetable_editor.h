#ifndef WAVETABLE_EDITOR_H_INCLUDED
#define WAVETABLE_EDITOR_H_INCLUDED

#include "grid_container.h"

namespace fmpire
{
class IntEditor;
class StateManager;
class WaveformEditor;
class WaveformSelector;
class WavetableCreator;

class WavetableEditor : public GridContainer
{
public:
	WavetableEditor(Widget* parent, StateManager& state_mgr);
	virtual ~WavetableEditor() noexcept;

	void select_oscillator(const size_t osc);

private:
	ScopedPointer<IntEditor> oscillator_selector;
	ScopedPointer<WaveformEditor> waveform_editor;
	ScopedPointer<WaveformSelector> waveform_selector;
	WavetableCreator* wavetable;

	StateManager& state_manager;
};

}; // namespace fmpire

#endif // WAVETABLE_EDITOR_H_INCLUDED
