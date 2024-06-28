#include "oscillator.h"

#include "defines.h"
#include "utils.h"

#include <cmath>
#include <string>

namespace fmpire
{

Oscillator::Oscillator() :
	active(false),
	volume(0.1),
	wavetable_position(0),
	detune(0.5),
	pan(0.5),
	note_shift(0),
	phase_offset(0),
	phase_random(0),
	unison_size(1),
	unison_detune(0),
	unison_spread(0),
	unison_phase_random(0)
{
}

Oscillator::~Oscillator() noexcept
{
}

float Oscillator::sample(const float phase,
						 const float wavetable_position) const
{
	// return wavetable.sample(wavetable_position, phase);
	return phase > 0.5 ? 1.0 : -1.0;
}

void Oscillator::set_state(const std::string& key, std::string_view& state)
{
	if (key == KEY_EVERYTHING)
	{
		if (!state.starts_with(OSC_DATA_STRING))
		{
			return;
		}
		state.remove_prefix(OSC_DATA_STRING.size());
		set_state(KEY_OSC_WAVETABLE, state);
		set_state(KEY_OSC_ACTIVE, state);
		set_state(KEY_OSC_VOLUME, state);
		set_state(KEY_OSC_WT_POS, state);
		set_state(KEY_OSC_DETUNE, state);
		set_state(KEY_OSC_PAN, state);
		set_state(KEY_OSC_NOTE_SHIFT, state);
		set_state(KEY_OSC_PHASE_OFFSET, state);
		set_state(KEY_OSC_PHASE_RANDOM, state);
		set_state(KEY_OSC_UNISON_SIZE, state);
		set_state(KEY_OSC_UNISON_DETUNE, state);
		set_state(KEY_OSC_UNISON_SPREAD, state);
		set_state(KEY_OSC_UNISON_PHASE_RANDOM, state);
	}
	else if (key == KEY_OSC_WAVETABLE)
	{
		wavetable_creator.set_state(KEY_WT_ALL, state);
	}
	else if (key == KEY_OSC_ACTIVE)
	{
		active = state.starts_with("1");
		state.remove_prefix(1);
	}
	else if (key == KEY_OSC_VOLUME)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&volume),
					  sizeof(volume));
	}
	else if (key == KEY_OSC_WT_POS)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&wavetable_position),
					  sizeof(wavetable_position));
	}
	else if (key == KEY_OSC_DETUNE)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&detune),
					  sizeof(detune));
	}
	else if (key == KEY_OSC_PAN)
	{
		decode_base32(state, reinterpret_cast<uint8_t*>(&pan), sizeof(pan));
	}
	else if (key == KEY_OSC_NOTE_SHIFT)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&note_shift),
					  sizeof(note_shift));
	}
	else if (key == KEY_OSC_PHASE_OFFSET)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&phase_offset),
					  sizeof(phase_offset));
	}
	else if (key == KEY_OSC_PHASE_RANDOM)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&phase_random),
					  sizeof(phase_random));
	}
	else if (key == KEY_OSC_UNISON_SIZE)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&unison_size),
					  sizeof(unison_size));
	}
	else if (key == KEY_OSC_UNISON_DETUNE)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&unison_detune),
					  sizeof(unison_detune));
	}
	else if (key == KEY_OSC_UNISON_SPREAD)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&unison_spread),
					  sizeof(unison_spread));
	}
	else if (key == KEY_OSC_UNISON_PHASE_RANDOM)
	{
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&unison_phase_random),
					  sizeof(unison_phase_random));
	}
}

std::string Oscillator::get_state() const
{
	std::string data(OSC_DATA_STRING);

	data += wavetable_creator.get_state();
	data += active ? "1" : "0";
	data += encode_base32(reinterpret_cast<const uint8_t*>(&volume),
						  sizeof(volume));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&wavetable_position),
						  sizeof(wavetable_position));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&detune),
						  sizeof(detune));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&pan), sizeof(pan));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&note_shift),
						  sizeof(note_shift));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&phase_offset),
						  sizeof(phase_offset));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&phase_random),
						  sizeof(phase_random));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&unison_size),
						  sizeof(unison_size));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&unison_detune),
						  sizeof(unison_detune));
	data += encode_base32(reinterpret_cast<const uint8_t*>(&unison_spread),
						  sizeof(unison_spread));
	data +=
		encode_base32(reinterpret_cast<const uint8_t*>(&unison_phase_random),
					  sizeof(unison_phase_random));

	return data;
}

} // namespace fmpire
