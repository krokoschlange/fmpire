#include "oscillator_settings.h"

#include "aspect_ratio_container.h"
#include "defines.h"
#include "image_button.h"
#include "label.h"
#include "state_manager.h"
#include "utils.h"
#include "wavetable.h"
#include "wavetable_creator.h"
#include "wavetable_view.h"

namespace fmpire
{

OscillatorSettings::OscillatorSettings(Widget* parent,
									   const size_t osc_index,
									   StateManager& state_mgr) :
	GridContainer(parent),
	index(osc_index),
	state_manager(state_mgr),
	wavetable(new Wavetable()),
	wavetable_creator(new WavetableCreator()),
	octave_shift(0),
	semi_shift(0)
{
	state_manager.add(this, index);

	add_row(3, theme->line_thin, 0, 0, 40);
	add_row(12, 0, 0, 0, 0);
	add_row(4, 0, 0, 0, 0);
	add_row(4, 0, 0, 0, 0);
	add_row(4, 0, theme->line_thin, 0, 0);

	add_column(12, theme->line_thin, 0, 0, 0);
	add_column(12, 0, 0, 0, 0);
	add_column(12, 0, 0, 0, 0);
	add_column(12, 0, theme->line_thin, 0, 0);

	active_square = new AspectRatioContainer(this);
	put(active_square, 0, 0);

	active = new ImageButton(active_square);
	active->set_toggle(true);
	active->set_callback(this);

	label = new Label(this);
	label->set_text("OSC " + std::to_string(index + 1));
	put(label, 0, 1, 1, 2);

	wavetable_view = new WavetableView(this, index, *wavetable, state_manager);
	put(wavetable_view, 1, 0, 1, 4);

	volume = new Knob(this);
	volume->set_callback(this);
	volume->set_default_value(0.5);
	volume->set_label("VOL");
	volume->set_tooltip("Volume");
	put(volume, 2, 0);

	wavetable_position = new Knob(this);
	wavetable_position->set_callback(this);
	wavetable_position->set_default_value(0);
	wavetable_position->set_label("WT");
	wavetable_position->set_tooltip("Wavetable Position");
	put(wavetable_position, 2, 1);

	detune = new Knob(this);
	detune->set_callback(this);
	detune->set_default_value(0.5);
	detune->set_label("DET");
	detune->set_tooltip("Detune", -24, 48, " semi");
	put(detune, 2, 2);

	pan = new Knob(this);
	pan->set_callback(this);
	pan->set_default_value(0.5);
	pan->set_label("PAN");
	pan->set_tooltip("Pan", -1, 2);
	put(pan, 2, 3);

	octave_offset = new IntEditor(this);
	octave_offset->set_callback(this);
	octave_offset->set_default_value(0);
	octave_offset->set_limits(-4, 4);
	octave_offset->set_tooltip("Octave Shift");
	octave_offset->set_label("OCT");
	put(octave_offset, 3, 2);

	semi_offset = new IntEditor(this);
	semi_offset->set_callback(this);
	semi_offset->set_default_value(0);
	semi_offset->set_limits(-12, 12);
	semi_offset->set_tooltip("Semitone Shift");
	semi_offset->set_label("SEMI");
	put(semi_offset, 3, 3);

	phase_offset = new Knob(this);
	phase_offset->set_callback(this);
	phase_offset->set_label("PH OFF");
	phase_offset->set_tooltip("Phase Offset");
	put(phase_offset, 3, 0);

	phase_random = new Knob(this);
	phase_random->set_callback(this);
	phase_random->set_label("PH RND");
	phase_random->set_tooltip("Phase Random");
	put(phase_random, 3, 1);

	unison_size = new IntEditor(this);
	unison_size->set_callback(this);
	unison_size->set_default_value(1);
	unison_size->set_limits(1, FMPIRE_MAX_UNISON_AMOUNT);
	unison_size->set_tooltip("Unison Size");
	unison_size->set_label("UNI");
	put(unison_size, 4, 0);

	unison_detune = new Knob(this);
	unison_detune->set_callback(this);
	unison_detune->set_label("U DET");
	unison_detune->set_tooltip("Unison Detune", 0, 1, "semi");
	put(unison_detune, 4, 1);

	unison_spread = new Knob(this);
	unison_spread->set_callback(this);
	unison_spread->set_label("U SPR");
	unison_spread->set_tooltip("Unison Spread");
	put(unison_spread, 4, 2);

	unison_phase_random = new Knob(this);
	unison_phase_random->set_callback(this);
	unison_phase_random->set_label("U RND");
	unison_phase_random->set_tooltip("Unison Phase Random");
	put(unison_phase_random, 4, 3);
}

OscillatorSettings::~OscillatorSettings() noexcept
{
}

void OscillatorSettings::set_state(std::string_view& state)
{
	if (!state.starts_with(OSC_DATA_STRING))
	{
		return;
	}
	state.remove_prefix(OSC_DATA_STRING.size());
	wavetable_creator->set_state(KEY_WT_ALL, state);
	float value = 0;
	active->set_pressed(state.starts_with("1"));
	state.remove_prefix(1);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	volume->set_value(value);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	wavetable_position->set_value(value);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	detune->set_value(value);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	pan->set_value(value);
	int note_shift = 0;
	decode_base32(state,
				  reinterpret_cast<uint8_t*>(&note_shift),
				  sizeof(note_shift));
	octave_shift = note_shift / 12;
	octave_offset->set_value(octave_shift);
	semi_shift = note_shift % 12;
	semi_offset->set_value(semi_shift);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	phase_offset->set_value(value);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	phase_random->set_value(value);
	uint32_t u_size;
	decode_base32(state, reinterpret_cast<uint8_t*>(&u_size), sizeof(u_size));
	unison_size->set_value(u_size);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	unison_detune->set_value(value);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	unison_spread->set_value(value);
	decode_base32(state, reinterpret_cast<uint8_t*>(&value), sizeof(value));
	unison_phase_random->set_value(value);
}

void OscillatorSettings::on_press(Button* const button)
{
	if (button == active)
	{
		state_manager.set_state(create_key(KEY_OSC_ACTIVE), "1");
	}
}

void OscillatorSettings::on_release(Button* const button)
{
	if (button == active)
	{
		state_manager.set_state(create_key(KEY_OSC_ACTIVE), "0");
	}
}

void OscillatorSettings::on_value_changed(IntEditor* const editor,
										  const int value)
{
	std::string data;
	std::string key;
	if (editor == octave_offset)
	{
		key = create_key(KEY_OSC_NOTE_SHIFT);
		octave_shift = value;
		int note_shift = octave_shift * 12 + semi_shift;
		data = encode_base32(reinterpret_cast<const uint8_t*>(&note_shift),
							 sizeof(note_shift));
	}
	else if (editor == semi_offset)
	{
		key = create_key(KEY_OSC_NOTE_SHIFT);
		semi_shift = value;
		int note_shift = octave_shift * 12 + semi_shift;
		data = encode_base32(reinterpret_cast<const uint8_t*>(&note_shift),
							 sizeof(note_shift));
	}
	else if (editor == unison_size)
	{
		key = create_key(KEY_OSC_UNISON_SIZE);
		data = encode_base32(reinterpret_cast<const uint8_t*>(&value),
							 sizeof(value));
	}
	if (!key.empty())
	{
		state_manager.set_state(key, data);
	}
}

void OscillatorSettings::drag_started(Knob* const knob)
{
}

void OscillatorSettings::drag_ended(Knob* const knob)
{
}

void OscillatorSettings::value_changed(Knob* const knob, const float value)
{
	std::string data =
		encode_base32(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
	std::string key = KEY_OSC_PREFIX + std::to_string(index) + "/";
	if (knob == volume)
	{
		key += KEY_OSC_VOLUME;
	}
	else if (knob == wavetable_position)
	{
		key += KEY_OSC_WT_POS;
	}
	else if (knob == detune)
	{
		key += KEY_OSC_DETUNE;
	}
	else if (knob == pan)
	{
		key += KEY_OSC_PAN;
	}
	else if (knob == phase_offset)
	{
		key += KEY_OSC_PHASE_OFFSET;
	}
	else if (knob == phase_random)
	{
		key += KEY_OSC_PHASE_RANDOM;
	}
	else if (knob == unison_detune)
	{
		key += KEY_OSC_UNISON_DETUNE;
	}
	else if (knob == unison_spread)
	{
		key += KEY_OSC_UNISON_SPREAD;
	}
	else if (knob == unison_phase_random)
	{
		key += KEY_OSC_UNISON_PHASE_RANDOM;
	}
	state_manager.set_state(key, data);
}

std::string OscillatorSettings::create_key(const std::string& subkey)
{
	return KEY_OSC_PREFIX + std::to_string(index) + "/" + subkey;
}

WavetableCreator* OscillatorSettings::get_wavetable() const
{
	return wavetable_creator;
}

} // namespace fmpire
