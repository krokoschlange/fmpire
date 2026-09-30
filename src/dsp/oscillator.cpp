#include "oscillator.h"

#include "defines.h"
#include "utils.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

namespace fmpire
{

OscillatorState::OscillatorState()
{
	rebuild_wavetable();
}

OscillatorState::~OscillatorState() noexcept
{
}

void OscillatorState::rebuild_wavetable()
{
	uint32_t width, height;
	wavetable_creator.get_size(width, height);

	std::shared_ptr<Wavetable> table = std::make_shared<Wavetable>();
	table->update(width, height, wavetable_creator.create_wavetable());
	wavetable = std::move(table);
}

void OscillatorState::set_state(const std::string_view& key, std::string_view& state)
{
	if (key == KEY_EVERYTHING)
	{
		if (!state.starts_with(OSC_DATA_STRING))
		{
			return;
		}
		state.remove_prefix(OSC_DATA_STRING.size());
		set_state(KEY_OSC_WAVETABLE KEY_WT_ALL, state);
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
	else if (key.starts_with(KEY_OSC_WAVETABLE))
	{
		std::string_view key_view = key;
		key_view.remove_prefix(strlen(KEY_OSC_WAVETABLE));
		wavetable_creator.set_state(std::string(key_view), state);
		rebuild_wavetable();
	}
	else if (key == KEY_OSC_ACTIVE)
	{
		if (state.empty())
		{
			return;
		}
		params.active = state.starts_with("1");
		state.remove_prefix(1);
	}
	else if (key == KEY_OSC_VOLUME)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.volume),
					  sizeof(params.volume));
		std::cout << params.volume << std::endl;
	}
	else if (key == KEY_OSC_WT_POS)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.wavetable_position),
					  sizeof(params.wavetable_position));
		std::cout << "wtpos " << params.wavetable_position << std::endl;
	}
	else if (key == KEY_OSC_DETUNE)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.detune),
					  sizeof(params.detune));
	}
	else if (key == KEY_OSC_PAN)
	{
		decode_base64(state, reinterpret_cast<uint8_t*>(&params.pan), sizeof(params.pan));
	}
	else if (key == KEY_OSC_NOTE_SHIFT)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.note_shift),
					  sizeof(params.note_shift));
	}
	else if (key == KEY_OSC_PHASE_OFFSET)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.phase_offset),
					  sizeof(params.phase_offset));
	}
	else if (key == KEY_OSC_PHASE_RANDOM)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.phase_random),
					  sizeof(params.phase_random));
	}
	else if (key == KEY_OSC_UNISON_SIZE)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.unison_size),
					  sizeof(params.unison_size));
	}
	else if (key == KEY_OSC_UNISON_DETUNE)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.unison_detune),
					  sizeof(params.unison_detune));
	}
	else if (key == KEY_OSC_UNISON_SPREAD)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.unison_spread),
					  sizeof(params.unison_spread));
	}
	else if (key.starts_with(KEY_OSC_MOD_DEPTH))
	{
		// "<type>/<modulator>"
		std::string_view rest = key;
		rest.remove_prefix(strlen(KEY_OSC_MOD_DEPTH));
		uint32_t type = 0;
		uint32_t modulator = 0;
		std::from_chars_result res =
			std::from_chars(rest.data(), rest.data() + rest.size(), type);
		if (res.ec != std::errc() || res.ptr == rest.data() + rest.size()
			|| type >= matrix_type_count)
		{
			return;
		}
		rest.remove_prefix(res.ptr - rest.data() + 1);
		res = std::from_chars(rest.data(), rest.data() + rest.size(), modulator);
		if (res.ec != std::errc() || modulator >= FMPIRE_OSC_COUNT)
		{
			return;
		}

		float depth = 0.0f;
		decode_base64(state, reinterpret_cast<uint8_t*>(&depth), sizeof(depth));
		params.depth[type][modulator] =
			std::isfinite(depth) ? std::clamp(depth, 0.0f, 1.0f) : 0.0f;
	}
	else if (key == KEY_OSC_UNISON_PHASE_RANDOM)
	{
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&params.unison_phase_random),
					  sizeof(params.unison_phase_random));
	}
}

std::string OscillatorState::get_state() const
{
	std::string data(OSC_DATA_STRING);

	data += wavetable_creator.get_state();
	data += params.active ? "1" : "0";
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.volume),
						  sizeof(params.volume));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.wavetable_position),
						  sizeof(params.wavetable_position));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.detune),
						  sizeof(params.detune));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.pan), sizeof(params.pan));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.note_shift),
						  sizeof(params.note_shift));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.phase_offset),
						  sizeof(params.phase_offset));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.phase_random),
						  sizeof(params.phase_random));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.unison_size),
						  sizeof(params.unison_size));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.unison_detune),
						  sizeof(params.unison_detune));
	data += encode_base64(reinterpret_cast<const uint8_t*>(&params.unison_spread),
						  sizeof(params.unison_spread));
	data +=
		encode_base64(reinterpret_cast<const uint8_t*>(&params.unison_phase_random),
					  sizeof(params.unison_phase_random));

	return data;
}

std::string OscillatorState::get_matrix_state() const
{
	std::string data;
	for (const auto& depths : params.depth)
	{
		for (const float depth : depths)
		{
			data += encode_base64(reinterpret_cast<const uint8_t*>(&depth),
								  sizeof(depth));
		}
	}
	return data;
}

void OscillatorState::set_matrix_state(std::string_view& state)
{
	for (auto& depths : params.depth)
	{
		for (float& depth : depths)
		{
			float value = 0.0f;
			decode_base64(state,
						  reinterpret_cast<uint8_t*>(&value),
						  sizeof(value));
			depth = std::isfinite(value) ? std::clamp(value, 0.0f, 1.0f) : 0.0f;
		}
	}
}

} // namespace fmpire
