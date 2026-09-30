
#include "fmpire.h"

#include "defines.h"
#include "utils.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <string>

namespace fmpire
{

FMpire::FMpire() :
	Plugin(FMPIRE_PARAMETER_COUNT, 0, 1),
	sync_time(false),
	self_frame(0),
	volume(1),
	voices(init_array<Voice, FMPIRE_VOICE_COUNT>(
		Voice(global_sources, this))),
	current_bpm(120.0f),
	last_started_voice(nullptr),
	modulator_generation(0)
{
	std::fill(voice_map.begin(), voice_map.end(), nullptr);
	for (std::atomic<float>& meter : route_meters)
	{
		meter.store(0.0f, std::memory_order_relaxed);
	}
	for (std::atomic<float>& playhead : playheads)
	{
		playhead.store(-1.0f, std::memory_order_relaxed);
	}
	for (size_t voice_idx = 0; voice_idx < voices.size(); voice_idx++)
	{
		free_voice_queue.push(&voices[voice_idx]);
	}

	oscillator_states[0].enable();

	// nothing else can see the plugin yet
	for (size_t index = 0; index < 3; index++)
	{
		build_patch(patches.initial_buffer(index));
	}
}

namespace
{
ModRoute make_route(const RouteSettings& settings)
{
	ModRoute route;
	route.active = settings.source.type != SourceType::NONE;
	route.amount = settings.amount;
	route.bipolar = settings.bipolar;
	route.source = settings.source;
	route.target = settings.target;
	route.target_object = settings.target_object;
	return route;
}

bool route_uses_modulator(const RouteSettings& route, const size_t id)
{
	const bool uses_as_source =
		route.source.type == SourceType::MODULATOR && route.source.index == id;
	const bool uses_as_target =
		is_modulator_target(route.target) && route.target_object == id;
	return uses_as_source || uses_as_target;
}
} // namespace

bool FMpire::load_modulator(const size_t id, std::string_view& state)
{
	if (id >= FMPIRE_ID_SPACE)
	{
		return false;
	}
	if (id >= modulator_shadows.size())
	{
		modulator_shadows.resize(id + 1);
	}

	ModulatorShadow& shadow = modulator_shadows[id];
	shadow.exists = true;
	shadow.settings.decode(state);
	shadow.curve.decode(state);
	update_modulator(id);

	shadow.modulator.set_generation(++modulator_generation);
	return true;
}

void FMpire::update_modulator(const size_t id)
{
	ModulatorShadow& shadow = modulator_shadows[id];

	const uint32_t generation = shadow.modulator.get_generation();
	shadow.modulator = Modulator(shadow.settings, shadow.curve);
	shadow.modulator.set_enabled(true);
	shadow.modulator.set_generation(generation);

	shadow.settings = shadow.modulator.get_settings();
}

void FMpire::remove_modulator(const size_t id)
{
	if (id >= modulator_shadows.size())
	{
		return;
	}

	modulator_shadows[id].exists = false;
	for (std::optional<RouteSettings>& route : route_shadows)
	{
		if (route && route_uses_modulator(*route, id))
		{
			route.reset();
		}
	}
}

bool FMpire::load_route(RouteSettings settings)
{
	if (settings.slot >= FMPIRE_ID_SPACE)
	{
		return false;
	}
	if (settings.slot >= route_shadows.size())
	{
		route_shadows.resize(settings.slot + 1);
	}

	if (settings.source.type == SourceType::NONE)
	{
		route_shadows[settings.slot].reset();
	}
	else
	{
		route_shadows[settings.slot] = settings;
	}
	return true;
}

void FMpire::set_modulator_state(std::string_view key, std::string_view& state)
{
	key.remove_prefix(strlen(KEY_MOD_PREFIX));

	uint32_t id = 0;
	std::from_chars_result res =
		std::from_chars(key.data(), key.data() + key.size(), id);
	if (res.ec != std::errc() || res.ptr == key.data() + key.size())
	{
		return;
	}
	key.remove_prefix(res.ptr - key.data() + 1);

	if (key == KEY_MOD_CREATE)
	{
		load_modulator(id, state);
	}
	else if (id < modulator_shadows.size() && modulator_shadows[id].exists)
	{
		ModulatorShadow& shadow = modulator_shadows[id];
		if (key == KEY_MOD_REMOVE)
		{
			remove_modulator(id);
		}
		else if (key == KEY_MOD_SETTINGS)
		{
			shadow.settings.decode(state);
			update_modulator(id);
		}
		else if (key == KEY_MOD_CURVE)
		{
			shadow.curve.decode(state);
			update_modulator(id);
		}
	}
}

void FMpire::set_route_state(std::string_view key, std::string_view& state)
{
	key.remove_prefix(strlen(KEY_ROUTE_PREFIX));

	uint32_t slot = 0;
	std::from_chars_result res =
		std::from_chars(key.data(), key.data() + key.size(), slot);
	if (res.ec != std::errc() || res.ptr == key.data() + key.size())
	{
		return;
	}
	key.remove_prefix(res.ptr - key.data() + 1);

	if (key == KEY_ROUTE_SET)
	{
		RouteSettings settings;
		settings.decode(state);
		settings.slot = slot;
		load_route(settings);
	}
	else if (slot < route_shadows.size() && route_shadows[slot])
	{
		if (key == KEY_ROUTE_AMOUNT)
		{
			float amount = 0.0f;
			decode_base64(state,
						  reinterpret_cast<uint8_t*>(&amount),
						  sizeof(amount));
			if (std::isfinite(amount))
			{
				route_shadows[slot]->amount = std::clamp(amount, -1.0f, 1.0f);
			}
		}
		else if (key == KEY_ROUTE_REMOVE)
		{
			route_shadows[slot].reset();
		}
	}
}

void FMpire::restore_everything(std::string_view& state)
{
	for (ModulatorShadow& shadow : modulator_shadows)
	{
		shadow.exists = false;
	}
	for (std::optional<RouteSettings>& route : route_shadows)
	{
		route.reset();
	}

	for (OscillatorState& oscillator : oscillator_states)
	{
		oscillator.set_state(KEY_EVERYTHING, state);
	}

	while (!state.empty())
	{
		if (state.starts_with(MODULATOR_DATA_STRING))
		{
			state.remove_prefix(MODULATOR_DATA_STRING.size());
			uint32_t id = 0;
			decode_base64(state, reinterpret_cast<uint8_t*>(&id), sizeof(id));

			if (!load_modulator(id, state))
			{
				return;
			}
		}
		else if (state.starts_with(ROUTE_DATA_STRING))
		{
			state.remove_prefix(ROUTE_DATA_STRING.size());
			RouteSettings settings;
			settings.decode(state);
			load_route(settings);
		}
		else if (state.starts_with(MATRIX_DATA_STRING))
		{
			state.remove_prefix(MATRIX_DATA_STRING.size());
			for (OscillatorState& oscillator : oscillator_states)
			{
				oscillator.set_matrix_state(state);
			}
		}
		else
		{
			return;
		}
	}
}

void FMpire::build_patch(Patch& patch) const
{
	for (size_t index = 0; index < oscillator_states.size(); index++)
	{
		patch.oscillators[index].params = oscillator_states[index].get_params();
		patch.oscillators[index].wavetable =
			oscillator_states[index].get_wavetable();
	}

	static const Modulator no_modulator;

	size_t modulator_count = modulator_shadows.size();
	while (modulator_count > 0 && !modulator_shadows[modulator_count - 1].exists)
	{
		modulator_count--;
	}
	patch.modulators.resize(modulator_count);
	for (size_t id = 0; id < modulator_count; id++)
	{
		patch.modulators[id] = modulator_shadows[id].exists
								 ? modulator_shadows[id].modulator
								 : no_modulator;
	}

	size_t route_count = route_shadows.size();
	while (route_count > 0 && !route_shadows[route_count - 1])
	{
		route_count--;
	}
	patch.routes.resize(route_count);
	for (size_t slot = 0; slot < route_count; slot++)
	{
		patch.routes[slot] = route_shadows[slot] ? make_route(*route_shadows[slot])
												 : ModRoute();
	}
}

FMpire::~FMpire() noexcept
{
}

const char* FMpire::getLabel() const
{
	return "FMpire";
}

const char* FMpire::getDescription() const
{
	return "FMpire";
}

const char* FMpire::getMaker() const
{
	return "krokoschlange";
}

const char* FMpire::getHomePage() const
{
	return "github.com/krokoschlange/fmpire";
}

const char* FMpire::getLicense() const
{
	return "MIT";
}

uint32_t FMpire::getVersion() const
{
	return d_version(0, 1, 0);
}

int64_t FMpire::getUniqueId() const
{
	return d_cconst('K', 'f', 'm', 'p');
}

void FMpire::initParameter(uint32_t index, Parameter& parameter)
{
	if (index >= FMPIRE_MACRO_COUNT)
	{
		if (index >= FMPIRE_PARAMETER_COUNT)
		{
			return;
		}

		const bool is_playhead = index >= FMPIRE_PLAYHEAD_BASE;
		const uint32_t number =
			(is_playhead ? index - FMPIRE_PLAYHEAD_BASE : index - FMPIRE_METER_BASE)
			+ 1;
		char meter_name[32];
		std::snprintf(meter_name,
					  sizeof(meter_name),
					  is_playhead ? "Playhead %u" : "Mod meter %u",
					  number);
		char meter_short_name[8];
		std::snprintf(meter_short_name,
					  sizeof(meter_short_name),
					  is_playhead ? "PH%u" : "MM%u",
					  number);
		char meter_symbol[32];
		std::snprintf(meter_symbol,
					  sizeof(meter_symbol),
					  is_playhead ? "playhead_%u" : "mod_meter_%u",
					  number);

		// -1..1: modulation offsets are signed, and a playhead of -1 means
		// the modulator isn't playing
		parameter.hints = kParameterIsOutput | kParameterIsHidden;
		parameter.name = meter_name;
		parameter.shortName = meter_short_name;
		parameter.symbol = meter_symbol;
		parameter.ranges.min = -1.0f;
		parameter.ranges.max = 1.0f;
		parameter.ranges.def = is_playhead ? -1.0f : 0.0f;
		return;
	}

	char name[16];
	std::snprintf(name, sizeof(name), "Macro %u", index + 1);
	char short_name[8];
	std::snprintf(short_name, sizeof(short_name), "M%u", index + 1);
	char symbol[16];
	std::snprintf(symbol, sizeof(symbol), "macro_%u", index + 1);

	parameter.hints = kParameterIsAutomatable;
	parameter.name = name;
	parameter.shortName = short_name;
	parameter.symbol = symbol;
	parameter.ranges.min = 0.0f;
	parameter.ranges.max = 1.0f;
	parameter.ranges.def = 0.0f;
}

float FMpire::getParameterValue(uint32_t index) const
{
	if (index >= FMPIRE_MACRO_COUNT)
	{
		if (index >= FMPIRE_PARAMETER_COUNT)
		{
			return 0.0f;
		}
		return index >= FMPIRE_PLAYHEAD_BASE
				 ? playheads[index - FMPIRE_PLAYHEAD_BASE].load(
					 std::memory_order_relaxed)
				 : route_meters[index - FMPIRE_METER_BASE].load(
					 std::memory_order_relaxed);
	}
	return global_sources.macros[index].load(std::memory_order_relaxed);
}

void FMpire::setParameterValue(uint32_t index, float value)
{
	if (index >= FMPIRE_MACRO_COUNT || !std::isfinite(value))
	{
		return;
	}
	global_sources.macros[index].store(std::clamp(value, 0.0f, 1.0f),
									   std::memory_order_relaxed);
}

void FMpire::initState(uint32_t index, State& state)
{
	if (index == 0)
	{
		state.hints = 0;
		state.key = KEY_EVERYTHING;
		state.defaultValue = getState(state.key);
		state.label = "everything";
		state.description = "stores entire plugin state";
		std::cout << state.key << std::endl;
	}
}

String FMpire::getState(const char* key) const
{
	std::string key_str(key);

	std::cout << "getting state (dsp) " << key << std::endl;
	String data;
	if (key_str == KEY_EVERYTHING)
	{
		const std::lock_guard<std::mutex> lock(state_mutex);

		for (const OscillatorState& oscillator : oscillator_states)
		{
			data += String(oscillator.get_state());
		}

		std::string matrix(MATRIX_DATA_STRING);
		for (const OscillatorState& oscillator : oscillator_states)
		{
			matrix += oscillator.get_matrix_state();
		}
		data += String(matrix);

		for (size_t id = 0; id < modulator_shadows.size(); id++)
		{
			const ModulatorShadow& shadow = modulator_shadows[id];
			if (!shadow.exists)
			{
				continue;
			}

			const uint32_t id_value = id;
			std::string section(MODULATOR_DATA_STRING);
			section += encode_base64(reinterpret_cast<const uint8_t*>(&id_value),
									 sizeof(id_value));
			section += shadow.settings.encode() + shadow.curve.encode();
			data += String(section);
		}

		for (size_t slot = 0; slot < route_shadows.size(); slot++)
		{
			if (!route_shadows[slot])
			{
				continue;
			}

			RouteSettings settings = *route_shadows[slot];
			settings.slot = slot;
			data += String(ROUTE_DATA_STRING + settings.encode());
		}
		std::cout << "GET EVERYTHING\n";
		std::cout << data;
		std::cout << std::endl;
	}
	return data;
}

void FMpire::setState(const char* key, const char* value)
{
	d_stdout("state set (dsp) %s %s", key, value);

	std::string_view key_view(key);
	std::string_view state(value);

	const std::lock_guard<std::mutex> lock(state_mutex);

	if (key_view == KEY_EVERYTHING)
	{
		restore_everything(state);
	}
	else if (key_view.starts_with(KEY_MOD_PREFIX))
	{
		set_modulator_state(key_view, state);
	}
	else if (key_view.starts_with(KEY_ROUTE_PREFIX))
	{
		set_route_state(key_view, state);
	}
	else if (key_view.starts_with(KEY_OSC_PREFIX))
	{
		key_view.remove_prefix(strlen(KEY_OSC_PREFIX));
		uint32_t index = 0;
		std::from_chars_result res =
			std::from_chars(key_view.data(),
							key_view.data() + key_view.size(),
							index);
		if (res.ec != std::errc() || res.ptr == key_view.data() + key_view.size()
			|| index >= oscillator_states.size())
		{
			return;
		}
		key_view.remove_prefix(res.ptr - key_view.data() + 1);

		oscillator_states[index].set_state(key_view, state);
	}

	build_patch(patches.write_buffer());
	patches.publish();
}

void FMpire::run(const float** inputs,
				 float** outputs,
				 uint32_t frames,
				 const MidiEvent* midiEvents,
				 uint32_t midiEventCount)
{
	float* const left = outputs[0];
	float* const right = outputs[1];

	std::fill(left, left + frames, 0);
	std::fill(right, right + frames, 0);

	const Patch& patch = patches.read();
	for (Voice& voice : voices)
	{
		voice.set_patch(patch);
	}

	const TimePosition& timepos = getTimePosition();
	if (sync_time)
	{
		self_frame = timepos.frame;
	}
	current_bpm = timepos.bbt.valid && timepos.bbt.beatsPerMinute > 1.0
					? (float) timepos.bbt.beatsPerMinute
					: 120.0f;

	for (size_t event_idx = 0; event_idx < midiEventCount; event_idx++)
	{
		on_midi_event(midiEvents[event_idx]);
	}

	for (size_t voice_idx = 0; voice_idx < voices.size(); voice_idx++)
	{
		voices[voice_idx].run(outputs, frames, current_bpm);
	}

	publish_meters(patch);
}

void FMpire::publish_meters(const Patch& patch)
{
	// the newest voice, or any voice that is still sounding
	const Voice* display_voice = nullptr;
	if (last_started_voice && last_started_voice->is_active())
	{
		display_voice = last_started_voice;
	}
	else
	{
		for (const Voice& voice : voices)
		{
			if (voice.is_active())
			{
				display_voice = &voice;
				break;
			}
		}
	}

	const size_t route_count = std::min(patch.routes.size(), route_meters.size());
	for (size_t slot = 0; slot < route_meters.size(); slot++)
	{
		float offset = 0.0f;
		if (display_voice && slot < route_count && patch.routes[slot].active)
		{
			const ModRoute& route = patch.routes[slot];
			offset = std::clamp(display_voice->get_modulation_offset(
									route.target,
									route.target_object),
								-1.0f,
								1.0f);
		}
		route_meters[slot].store(offset, std::memory_order_relaxed);
	}

	for (size_t id = 0; id < playheads.size(); id++)
	{
		playheads[id].store(display_voice ? display_voice->get_playhead(id) : -1.0f,
							std::memory_order_relaxed);
	}
}

void FMpire::on_midi_event(const MidiEvent& event)
{
	uint8_t message_type = event.data[0] >> 4;
	uint8_t channel = event.data[0] & 0b1111;
	uint32_t offset = event.frame;
	switch (message_type)
	{
	case 0b1000:
	{
		int note = event.data[1] & 0b01111111;
		on_note_off(offset, note);
		break;
	}
	case 0b1001:
	{
		int note = event.data[1] & 0b01111111;
		float velocity = (event.data[2] & 0b01111111) / 127.0f;
		on_note_on(offset, note, velocity);
		break;
	}
	case 0b1010:
	{
		int note = event.data[1] & 0b01111111;
		float pressure = (event.data[2] & 0b01111111) / 127.0f;
		on_poly_aftertouch(offset, note, pressure);
		break;
	}
	case 0b1011:
	{
		on_midi_control(offset, event.data[1], event.data[2]);
		break;
	}
	case 0b1101:
	{
		float pressure = (event.data[1] & 0b01111111) / 127.0f;
		on_mono_aftertouch(offset, pressure);
		break;
	}
	case 0b1110:
	{
		uint16_t int_val = (event.data[1] & 0b01111111)
						 | (((uint16_t) (event.data[2] & 0b01111111)) << 7);
		float value = (float) int_val / 16383.0f;
		on_pitch_wheel_change(offset, value);
		break;
	}
	default:
		break;
	}
}

void FMpire::on_note_off(const uint32_t offset, const int note)
{
	if (voice_map[note])
	{
		Voice* voice = voice_map[note];
		voice->stop(offset);
	}
}

void FMpire::on_note_on(const uint32_t offset,
						const int note,
						const float velocity)
{
	// A retriggered note lets the old voice ring out its release (cutting it
	// off would click, the envelope is usually not at zero yet); it just stops
	// being the voice of the note.
	if (Voice* previous = voice_map[note])
	{
		if (!previous->is_stopping())
		{
			previous->stop(offset);
		}
		voice_map[note] = nullptr;
	}

	Voice* voice = nullptr;
	if (free_voice_queue.empty())
	{
		voice = &voices[0];
	}
	else
	{
		voice = free_voice_queue.front();
		free_voice_queue.pop();
	}

	if (voice->is_active())
	{
		voice->kill();
	}
	voice_map[note] = voice;
	voice->start(offset, note, velocity, getSampleRate(), current_bpm);
	last_started_voice = voice;
}

void FMpire::on_poly_aftertouch(const uint32_t offset,
								const int note,
								const float pressure)
{
	if (voice_map[note])
	{
		voice_map[note]->set_poly_pressure(pressure);
	}
}

void FMpire::on_mono_aftertouch(const uint32_t offset, const float pressure)
{
	global_sources.channel_pressure = pressure;
}

void FMpire::on_pitch_wheel_change(const uint32_t offset, const float value)
{
	global_sources.pitch_bend = value;
}

void FMpire::on_midi_control(const uint32_t offset,
							 const uint8_t message,
							 const uint8_t value)
{
	const uint8_t controller = message & 0b01111111;
	global_sources.cc[controller] = (value & 0b01111111) / 127.0f;

	switch (controller)
	{
	case 120:
		on_all_sound_off();
		break;
	case 123:
		on_all_notes_off();
		break;
	default:
		break;
	}
}

void FMpire::on_all_sound_off()
{
	for (Voice& voice : voices)
	{
		voice.kill();
	}
	std::fill(voice_map.begin(), voice_map.end(), nullptr);
}

void FMpire::on_all_notes_off()
{
	for (Voice& voice : voices)
	{
		voice.stop(0);
	}
}

void FMpire::on_voice_ended(Voice* const voice)
{
	// a voice that was retriggered no longer owns its note
	if (voice_map[voice->get_note()] == voice)
	{
		voice_map[voice->get_note()] = nullptr;
	}
	free_voice_queue.push(voice);
}

} // namespace fmpire


