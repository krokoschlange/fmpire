#include "voice.h"

#include "modulator.h"
#include "oscillator.h"

#include "utils.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <string.h>

namespace fmpire
{

OscillatorVoice::OscillatorVoice() :
	oscillator(nullptr),
	samplerate(44100.0f),
	last_unison_size(0),
	last_sample(0.0f),
	reference_index(0),
	base_frequency(440.0f),
	volume(0.0f),
	wavetable_position(0.0f),
	detune(0.5f),
	pan(0.5f),
	note_shift(0),
	unison_detune(0.0f),
	unison_spread(0.0f)
{
	unison_phases.fill(0.0f);
	unison_detunes.fill(0.0f);
	unison_pans.fill(0.5f);
	mod_offsets.fill(0.0f);
}

OscillatorVoice::~OscillatorVoice() noexcept
{
}

void OscillatorVoice::init(std::default_random_engine& rand,
						   const int note,
						   const float rate)
{
	samplerate = rate;

	std::uniform_real_distribution<float> dist(0.0f, 1.0f);
	float r = dist(rand);
	float phase_random = r * oscillator->params.phase_random;
	for (size_t unison_voice = 0; unison_voice < FMPIRE_MAX_UNISON_AMOUNT;
		 unison_voice++)
	{
		float unison_phase_random =
			dist(rand) * oscillator->params.unison_phase_random;
		float phase =
			oscillator->params.phase_offset + phase_random + unison_phase_random;
		phase = std::fmod(phase, 1.0f);
		unison_phases[unison_voice] = phase;
	}

	base_frequency = 440.0f * std::exp2f((float) (note - 69) / 12.0f);

	last_unison_size = 0;
	calculate_unison_parameters();

	refresh_base_values();
	clear_modulation();
}

float OscillatorVoice::get_osc_value() const
{
	return last_sample;
}

void OscillatorVoice::calculate_unison_parameters()
{
	if (last_unison_size == oscillator->params.unison_size)
	{
		return;
	}
	reference_index = 0;
	float min_detune = 1.0f;
	for (size_t unison_voice = 0; unison_voice < oscillator->params.unison_size;
		 unison_voice++)
	{
		unison_pans[unison_voice] =
			(float) unison_voice / (float) (oscillator->params.unison_size - 1);

		if (oscillator->params.unison_size == 1)
		{
			unison_pans[unison_voice] = 0.5;
		}
		if (unison_voice <= (oscillator->params.unison_size - 1) / 2)
		{
			unison_detunes[unison_voice] = unison_voice * 2;
		}
		else
		{
			unison_detunes[unison_voice] =
				(oscillator->params.unison_size - 1 - unison_voice) * 2 + 1;
		}
		unison_detunes[unison_voice] /= (float) (oscillator->params.unison_size - 1);
		if (oscillator->params.unison_size == 1)
		{
			unison_detunes[unison_voice] = 0.5f;
		}
		unison_detunes[unison_voice] =
			unison_detunes[unison_voice] * 2.0f - 1.0f;

		if (std::fabs(unison_detunes[unison_voice]) < min_detune)
		{
			min_detune = std::fabs(unison_detunes[unison_voice]);
			reference_index = unison_voice;
		}
	}
	last_unison_size = oscillator->params.unison_size;
}

void OscillatorVoice::run_one_sample(float& left, float& right)
{
	auto modulated = [this](const float base, const TargetType target)
	{
		return std::clamp(base + mod_offsets[static_cast<size_t>(target)],
						  0.0f,
						  1.0f);
	};

	const float volume_now = modulated(volume, TargetType::OSC_VOLUME);
	const float wavetable_position_now =
		modulated(wavetable_position, TargetType::OSC_WT_POS);
	const float detune_now = modulated(detune, TargetType::OSC_DETUNE);
	const float pan_now = modulated(pan, TargetType::OSC_PAN);
	const float unison_detune_now =
		modulated(unison_detune, TargetType::OSC_UNISON_DETUNE);
	const float unison_spread_now =
		modulated(unison_spread, TargetType::OSC_UNISON_SPREAD);

	for (size_t unison_voice = 0; unison_voice < oscillator->params.unison_size;
		 unison_voice++)
	{
		float smpl = oscillator->sample(unison_phases[unison_voice],
										wavetable_position_now);

		if (unison_voice == reference_index)
		{
			last_sample = smpl;
		}

		smpl *= volume_now;

		float unison_pan = unison_pans[unison_voice];
		unison_pan = lerp(0.5f, unison_pan, unison_spread_now);
		float left_panned = smpl * cosf(unison_pan * M_PI_2);
		float right_panned = smpl * sinf(unison_pan * M_PI_2);

		if (pan_now < 0.5)
		{
			right_panned *= pan_now * 2.0f;
		}
		else
		{
			left_panned *= (1.0f - pan_now) * 2.0f;
		}
		left += left_panned;
		right += right_panned;

		float uni_det = unison_detunes[unison_voice];
		uni_det *= unison_detune_now / 12.0f;

		float detune_shift =
			-2.0f + 4.0f * detune_now + uni_det + (float) note_shift / 12.0f;
		float detune_factor = std::exp2f(detune_shift);
		float freq = base_frequency * detune_factor;
		float& phase = unison_phases[unison_voice];
		phase += freq / samplerate;
		phase = std::fmod(phase, 1.0f);
	}
}

void OscillatorVoice::refresh_base_values()
{
	volume = oscillator->params.volume;
	wavetable_position = oscillator->params.wavetable_position;
	detune = oscillator->params.detune;
	pan = oscillator->params.pan;
	note_shift = oscillator->params.note_shift;
	unison_detune = oscillator->params.unison_detune;
	unison_spread = oscillator->params.unison_spread;
}

namespace
{
// Modulators every voice has playback state for from the start; more are
// added when a patch needs them.
constexpr size_t initial_modulator_capacity = 64;

// Time constant for smoothing the jump when an envelope's release starts from
// the level it was at when the key was let go.
constexpr float release_fade_seconds = 0.004f;

// The speed-modulation range: a full-scale contribution changes the LFO/ENV
// speed by this many octaves.
constexpr float speed_modulation_octaves = 4.0f;
} // namespace

ModulatorVoice::ModulatorVoice() :
	generation(0),
	samplerate(44100.0f),
	release_decay(0.99f),
	position(0.0f),
	released(false),
	value(0.0f),
	gain(0.0f),
	release_offset(0.0f),
	amount_offset(0.0f),
	speed_offset(0.0f)
{
}

void ModulatorVoice::init(const Modulator& mod, const float rate)
{
	generation = mod.get_generation();
	samplerate = rate;
	release_decay = std::exp(-1.0f / (release_fade_seconds * rate));

	position = 0.0f;
	released = false;
	release_offset = 0.0f;
	clear_modulation();

	gain = mod.get_amount();
	value = Curve::lookup(mod.get_table(),
						  mod.get_type() == Modulator::Type::LFO
							  ? mod.get_phase_offset()
							  : 0.0f);
}

void ModulatorVoice::process(const Modulator& mod, const float bpm)
{
	const float* table = mod.get_table();

	gain = std::clamp(mod.get_amount() + amount_offset, 0.0f, 1.0f);

	const float speed = std::exp2(std::clamp(speed_offset, -1.0f, 1.0f)
								  * speed_modulation_octaves);
	const float step = speed / (mod.get_length_seconds(bpm) * samplerate);

	if (mod.get_type() == Modulator::Type::LFO)
	{
		float phase = position + mod.get_phase_offset();
		phase -= std::floor(phase);
		value = Curve::lookup(table, phase);

		position += step;
		position -= std::floor(position);
		return;
	}

	value = std::clamp(Curve::lookup(table, position) + release_offset,
					   0.0f,
					   1.0f);
	release_offset *= release_decay;

	// hold at the sustain point until note off, then play out to the end
	const float limit = released ? 1.0f : mod.get_sustain_pos();
	position = std::min(position + step, limit);
}

void ModulatorVoice::release(const Modulator& mod)
{
	if (released)
	{
		return;
	}
	released = true;

	if (mod.get_type() != Modulator::Type::ENVELOPE)
	{
		return;
	}

	// Continue from the point of the release section that matches the current
	// level, so letting go early (or late) doesn't jump.
	const float* table = mod.get_table();
	const float level = value;

	size_t index = std::min(
		static_cast<size_t>(mod.get_sustain_pos() * Curve::table_size),
		Curve::table_size);
	while (index < Curve::table_size && table[index] > level)
	{
		index++;
	}

	position = static_cast<float>(index) / Curve::table_size;
	release_offset = level - table[index];
}

void ModulatorVoice::add_modulation(const TargetType target, const float amount)
{
	if (target == TargetType::MOD_AMOUNT)
	{
		amount_offset += amount;
	}
	else if (target == TargetType::MOD_FREQ)
	{
		speed_offset += amount;
	}
}

Voice::Voice(GlobalSources& global_sources, VoiceEndedCallback* ended_cb) :
	active(false),
	patch(nullptr),
	globals(global_sources),
	modulator_voices(initial_modulator_capacity),
	modulators_released(false),
	samplerate(44100.0f),
	rand((size_t) this),
	ended_callback(ended_cb),
	volume(1)
{
}

Voice::Voice(const Voice& voice) :
	active(false),
	patch(nullptr),
	globals(voice.globals),
	modulator_voices(initial_modulator_capacity),
	modulators_released(false),
	samplerate(44100.0f),
	rand((size_t) this),
	ended_callback(voice.ended_callback),
	volume(1)
{
}

Voice::~Voice() noexcept
{
}

void Voice::set_patch(const Patch& new_patch)
{
	patch = &new_patch;

	for (size_t index = 0; index < oscillator_voices.size(); index++)
	{
		oscillator_voices[index].set_oscillator(&new_patch.oscillators[index]);
	}

	if (modulator_voices.size() < new_patch.modulators.size())
	{
		modulator_voices.resize(new_patch.modulators.size());
	}
}

void Voice::start(const size_t offset,
				  const int note_idx,
				  const float vol,
				  const float rate,
				  const float bpm)
{
	active = true;
	delay = offset;
	age = 0;
	note = note_idx;
	volume = vol;
	samplerate = rate;

	sources.velocity = vol;
	sources.key = std::clamp(note_idx, 0, 127) / 127.0f;
	sources.poly_pressure = 0.0f;

	for (OscillatorVoice& osc_voice : oscillator_voices)
	{
		osc_voice.init(rand, note, rate);
	}

	death_time = 0;

	const std::vector<Modulator>& modulators = patch->modulators;
	const size_t mod_count = modulators.size();
	for (size_t mod_idx = 0; mod_idx < mod_count; mod_idx++)
	{
		const Modulator& mod = modulators[mod_idx];
		modulator_voices[mod_idx].init(mod, rate);
		if (mod.is_enabled())
		{
			death_time = std::max(
				death_time,
				(size_t) (mod.get_release_seconds(bpm) * rate));
		}
	}
	modulators_released = false;

	stop_voice = false;
}

void Voice::stop(const size_t delay)
{
	stop_voice = true;
	stop_delay = delay;
}

void Voice::kill()
{
	active = false;
	ended_callback->on_voice_ended(this);
}

bool Voice::read_source(const SourceId& source,
						const size_t mod_count,
						float& value,
						float& gain) const
{
	gain = 1.0f;

	switch (source.type)
	{
	case SourceType::MODULATOR:
		if (source.index >= mod_count
			|| !patch->modulators[source.index].is_enabled())
		{
			return false;
		}
		value = modulator_voices[source.index].get_value();
		gain = modulator_voices[source.index].get_gain();
		return true;
	case SourceType::MACRO:
		if (source.index >= FMPIRE_MACRO_COUNT)
		{
			return false;
		}
		value = globals.macros[source.index].load(std::memory_order_relaxed);
		return true;
	case SourceType::MIDI_CC:
		if (source.index >= globals.cc.size())
		{
			return false;
		}
		value = globals.cc[source.index];
		return true;
	case SourceType::PITCH_BEND:
		value = globals.pitch_bend;
		return true;
	case SourceType::CHANNEL_PRESSURE:
		value = globals.channel_pressure;
		return true;
	case SourceType::POLY_PRESSURE:
		value = sources.poly_pressure;
		return true;
	case SourceType::VELOCITY:
		value = sources.velocity;
		return true;
	case SourceType::KEY:
		value = sources.key;
		return true;
	default:
		return false;
	}
}

void Voice::apply_routes(const size_t mod_count, const size_t route_count)
{
	for (size_t route_idx = 0; route_idx < route_count; route_idx++)
	{
		const ModRoute& route = patch->routes[route_idx];
		if (!route.active)
		{
			continue;
		}

		float value = 0.0f;
		float gain = 1.0f;
		if (!read_source(route.source, mod_count, value, gain))
		{
			continue;
		}

		const float centered = route.bipolar ? 2.0f * value - 1.0f : value;
		const float contribution = route.amount * gain * centered;

		if (static_cast<size_t>(route.target) < osc_target_count)
		{
			if (route.target_object < FMPIRE_OSC_COUNT)
			{
				oscillator_voices[route.target_object].add_modulation(
					route.target,
					contribution);
			}
		}
		else if (route.target_object < mod_count)
		{
			modulator_voices[route.target_object].add_modulation(route.target,
																 contribution);
		}
	}
}

void Voice::run(float** inout, size_t count, const float bpm)
{
	if (!active)
	{
		return;
	}

	float* left = inout[0];
	float* right = inout[1];

	// The patch may have changed since the last block: follow the knobs, and
	// restart modulators that were created (or replaced) since the state of
	// this voice was set up.
	const std::vector<Modulator>& modulators = patch->modulators;
	const size_t mod_count = modulators.size();
	const size_t route_count = patch->routes.size();

	for (OscillatorVoice& osc_voice : oscillator_voices)
	{
		osc_voice.refresh_base_values();
		osc_voice.calculate_unison_parameters();
	}
	for (size_t mod_idx = 0; mod_idx < mod_count; mod_idx++)
	{
		const Modulator& mod = modulators[mod_idx];
		if (mod.is_enabled()
			&& modulator_voices[mod_idx].get_generation() != mod.get_generation())
		{
			modulator_voices[mod_idx].init(mod, samplerate);
		}
	}

	for (size_t smpl = delay; smpl < count; smpl++)
	{
		// Modulators use the offsets their routes accumulated on the previous
		// sample (so modulators can modulate each other without ordering or
		// cycle problems, at one sample of latency), everything else gets this
		// sample's freshly accumulated offsets.
		for (size_t mod_idx = 0; mod_idx < mod_count; mod_idx++)
		{
			const Modulator& mod = modulators[mod_idx];
			if (mod.is_enabled())
			{
				modulator_voices[mod_idx].process(mod, bpm);
			}
		}

		for (OscillatorVoice& osc_voice : oscillator_voices)
		{
			osc_voice.clear_modulation();
		}
		for (size_t mod_idx = 0; mod_idx < mod_count; mod_idx++)
		{
			modulator_voices[mod_idx].clear_modulation();
		}
		apply_routes(mod_count, route_count);

		for (OscillatorVoice& osc_voice : oscillator_voices)
		{
			if (osc_voice.is_active())
			{
				float left_smpl = 0, right_smpl = 0;
				osc_voice.run_one_sample(left_smpl, right_smpl);
				left[smpl] += left_smpl * volume;
				right[smpl] += right_smpl * volume;
				float osc_sample = osc_voice.get_osc_value();
			}
		}

		if (stop_voice && stop_delay <= smpl)
		{
			if (!modulators_released)
			{
				for (size_t mod_idx = 0; mod_idx < mod_count; mod_idx++)
				{
					modulator_voices[mod_idx].release(modulators[mod_idx]);
				}
				modulators_released = true;
			}
			stop_delay = 0;
			if (death_time == 0)
			{
				active = false;
				ended_callback->on_voice_ended(this);
				return;
			}
			death_time--;
		}
	}

	if (delay > count)
	{
		delay -= count;
	}
	else
	{
		delay = 0;
	}
}

bool Voice::is_active()
{
	return active;
}

int Voice::get_note() const
{
	return note;
}


} // namespace fmpire
