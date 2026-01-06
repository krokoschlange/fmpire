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

	WavetableCreator* get_wavetable_creator() const;
	Wavetable* get_wavetable() const;

	void on_wavetable_changed() const;

private:
	size_t index;
	StateManager& state_manager;
	Ref<Wavetable> wavetable;
	Ref<WavetableCreator> wavetable_creator;
	Ref<AspectRatioContainer> active_square;
	Ref<Button> active;
	Ref<Label> label;
	Ref<WavetableView> wavetable_view;
	Ref<Knob> volume;
	Ref<Knob> wavetable_position;
	Ref<Knob> detune;
	Ref<Knob> pan;
	Ref<IntEditor> octave_offset;
	Ref<IntEditor> semi_offset;
	Ref<Knob> phase_offset;
	Ref<Knob> phase_random;
	Ref<IntEditor> unison_size;
	Ref<Knob> unison_detune;
	Ref<Knob> unison_spread;
	Ref<Knob> unison_phase_random;

	std::string create_key(const std::string& subkey);
	int octave_shift;
	int semi_shift;
};


} // namespace fmpire

#endif // OSCILLATOR_SETTINGS_H_INCLUDED
