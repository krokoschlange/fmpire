#ifndef OSCILLATOR_SETTINGS_H_INCLUDED
#define OSCILLATOR_SETTINGS_H_INCLUDED

#include "button.h"
#include "grid_container.h"
#include "int_editor.h"
#include "knob.h"

namespace fmpire
{
class AspectRatioContainer;
class Label;
class StateManager;
class Wavetable;
class WavetableCreator;
class WavetableView;

class OscillatorSettings :
	public GridContainer,
	public Button::Callback,
	public IntEditor::Callback,
	public Knob::Callback
{
public:
	OscillatorSettings(Widget* parent,
					   const size_t osc_index,
					   StateManager& state_mgr);
	virtual ~OscillatorSettings() noexcept;

	void set_state(std::string_view& state);

	virtual void on_press(Button* const button) override;
	virtual void on_release(Button* const button) override;

	virtual void on_value_changed(IntEditor* const editor,
								  const int value) override;

	virtual void drag_started(Knob* const knob) override;
	virtual void drag_ended(Knob* const knob) override;
	virtual void value_changed(Knob* const knob, const float value) override;

	WavetableCreator* get_wavetable() const;

private:
	size_t index;
	StateManager& state_manager;
	ScopedPointer<Wavetable> wavetable;
	ScopedPointer<WavetableCreator> wavetable_creator;
	ScopedPointer<AspectRatioContainer> active_square;
	ScopedPointer<Button> active;
	ScopedPointer<Label> label;
	ScopedPointer<WavetableView> wavetable_view;
	ScopedPointer<Knob> volume;
	ScopedPointer<Knob> wavetable_position;
	ScopedPointer<Knob> detune;
	ScopedPointer<Knob> pan;
	ScopedPointer<IntEditor> octave_offset;
	ScopedPointer<IntEditor> semi_offset;
	ScopedPointer<Knob> phase_offset;
	ScopedPointer<Knob> phase_random;
	ScopedPointer<IntEditor> unison_size;
	ScopedPointer<Knob> unison_detune;
	ScopedPointer<Knob> unison_spread;
	ScopedPointer<Knob> unison_phase_random;

	std::string create_key(const std::string& subkey);
	int octave_shift;
	int semi_shift;
};


} // namespace fmpire

#endif // OSCILLATOR_SETTINGS_H_INCLUDED
