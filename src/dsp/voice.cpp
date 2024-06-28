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

OscillatorVoice::OscillatorVoice(Oscillator& osc) :
	oscillator(&osc)
{
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
	float phase_random = r * oscillator->phase_random;
	for (size_t unison_voice = 0; unison_voice < FMPIRE_MAX_UNISON_AMOUNT;
		 unison_voice++)
	{
		float unison_phase_random =
			dist(rand) * oscillator->unison_phase_random;
		float phase =
			oscillator->phase_offset + phase_random + unison_phase_random;
		phase = std::fmod(phase, 1.0f);
		unison_phases[unison_voice] = phase;
	}

	base_frequency = 440.0f * std::exp2f((float) (note - 69) / 12.0f);

	last_unison_size = 0;
	calculate_unison_parameters();

	volume = oscillator->volume;
	wavetable_position = oscillator->wavetable_position;
	detune = oscillator->detune;
	pan = oscillator->pan;
	note_shift = oscillator->note_shift;
	unison_detune = oscillator->unison_detune;
	unison_spread = oscillator->unison_spread;
}

float OscillatorVoice::get_osc_value() const
{
	return last_sample;
}

void OscillatorVoice::calculate_unison_parameters()
{
	if (last_unison_size == oscillator->unison_size)
	{
		return;
	}
	reference_index = 0;
	float min_detune = 1.0f;
	for (size_t unison_voice = 0; unison_voice < oscillator->unison_size;
		 unison_voice++)
	{
		unison_pans[unison_voice] =
			(float) unison_voice / (float) (oscillator->unison_size - 1);

		if (oscillator->unison_size == 1)
		{
			unison_pans[unison_voice] = 0.5;
		}
		if (unison_voice <= (oscillator->unison_size - 1) / 2)
		{
			unison_detunes[unison_voice] = unison_voice * 2;
		}
		else
		{
			unison_detunes[unison_voice] =
				(oscillator->unison_size - 1 - unison_voice) * 2 + 1;
		}
		unison_detunes[unison_voice] /= (float) (oscillator->unison_size - 1);
		if (oscillator->unison_size == 1)
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
	last_unison_size = oscillator->unison_size;
}

void OscillatorVoice::run_one_sample(float& left, float& right)
{
	for (size_t unison_voice = 0; unison_voice < oscillator->unison_size;
		 unison_voice++)
	{
		float smpl =
			oscillator->sample(unison_phases[unison_voice], wavetable_position);

		if (unison_voice == reference_index)
		{
			last_sample = smpl;
		}

		smpl *= volume;

		float unison_pan = unison_pans[unison_voice];
		unison_pan = lerp(0.5f, unison_pan, unison_spread);
		float left_panned = smpl * cosf(unison_pan * M_PI_2);
		float right_panned = smpl * sinf(unison_pan * M_PI_2);

		if (pan < 0.5)
		{
			right_panned *= pan * 2.0f;
		}
		else
		{
			left_panned *= (1.0f - pan) * 2.0f;
		}
		left += left_panned;
		right += right_panned;

		float uni_det = unison_detunes[unison_voice];
		uni_det *= unison_detune / 12.0f;

		float detune_shift =
			-2.0f + 4.0f * detune + uni_det + (float) note_shift / 12.0f;
		float detune_factor = std::exp2f(detune_shift);
		float freq = base_frequency * detune_factor;
		float& phase = unison_phases[unison_voice];
		phase += freq / samplerate;
		phase = std::fmod(phase, 1.0f);
	}
}

void OscillatorVoice::set_state(const std::string& key, std::string_view& state)
{
	if (key == KEY_OSC_VOLUME)
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
}

ModulatorVoice::ModulatorVoice()
{
}

ModulatorVoice::~ModulatorVoice() noexcept
{
}

void ModulatorVoice::init(std::shared_ptr<Modulator> mod,
						  const ModulatorTargetObjects& target_objects,
						  const float rate)
{
	modulator = mod;
	targets.resize(modulator->targets.size());
	for (size_t target_idx = 0; target_idx < modulator->targets.size();
		 target_idx++)
	{
		targets[target_idx] =
			modulator->targets[target_idx].resolve(target_objects);
	}

	samplerate = rate;

	amount = modulator->amount;
	frequency = modulator->frequency;
	phase = 0.0f;
	released = false;
	frames_since_start = 0;
	frames_since_release = 0;
}

void ModulatorVoice::modulate()
{
	float sample = modulator->sample();
	std::vector<ModulatorTarget>& target_datas = modulator->targets;
	for (size_t target = 0; target < target_datas.size(); target++)
	{
		*targets[target] =
			lerp(target_datas[target].min, target_datas[target].max, sample);
	}
	frames_since_start++;
	if (released)
	{
		frames_since_release++;
	}
}

void ModulatorVoice::release()
{
	released = true;
}

Voice::Voice(std::array<fmpire::Oscillator, FMPIRE_OSC_COUNT>& oscs,
			 std::vector<std::shared_ptr<fmpire::Modulator>>& mods,
			 VoiceEndedCallback* ended_cb) :
	active(false),
	oscillators(oscs),
	modulators(mods),
	oscillator_voices(init_array_from_args_array<OscillatorVoice>(oscs)),
	rand((size_t) this),
	ended_callback(ended_cb),
	volume(1)
{
}

Voice::Voice(const Voice& voice) :
	active(false),
	oscillators(voice.oscillators),
	modulators(voice.modulators),
	oscillator_voices(init_array_from_args_array<OscillatorVoice>(oscillators)),
	rand((size_t) this),
	ended_callback(voice.ended_callback),
	volume(1)
{
}

Voice::~Voice() noexcept
{
}

void Voice::start(const size_t offset,
				  const int note_idx,
				  const float vol,
				  const float rate)
{
	active = true;
	delay = offset;
	age = 0;
	note = note_idx;
	volume = vol;
	for (OscillatorVoice& osc_voice : oscillator_voices)
	{
		osc_voice.init(rand, note, rate);
	}

	death_time = 0;

	modulator_voices.resize(modulators.size());
	for (size_t mod_idx = 0; mod_idx < modulators.size(); mod_idx++)
	{
		modulator_voices[mod_idx].init(modulators[mod_idx],
									   {oscillator_voices, modulator_voices},
									   rate);
		death_time =
			std::max(death_time,
					 (size_t) (modulators[mod_idx]->get_release() * rate));
	}
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

void Voice::run(float** inout, size_t count)
{
	if (!active)
	{
		return;
	}

	float* left = inout[0];
	float* right = inout[1];
	for (size_t osc_voice = 0; osc_voice < FMPIRE_OSC_COUNT; osc_voice++)
	{
		oscillator_voices[osc_voice].calculate_unison_parameters();
	}
	for (size_t smpl = delay; smpl < count; smpl++)
	{
		for (ModulatorVoice& mod_voice : modulator_voices)
		{
			mod_voice.modulate();
		}
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
			for (ModulatorVoice& mod_voice : modulator_voices)
			{
				mod_voice.release();
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

void Voice::set_state(std::string_view& key, std::string_view& state)
{
	if (key.starts_with(KEY_OSC_PREFIX))
	{
		key.remove_prefix(strlen(KEY_OSC_PREFIX));
		uint32_t index = 0;
		std::from_chars_result res =
			std::from_chars(key.data(), key.data() + key.size(), index);
		key.remove_prefix(res.ptr - key.data() + 1);
		index = std::clamp<uint32_t>(index, 0, oscillator_voices.size());
		oscillator_voices[index].set_state(std::string(key), state);
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
