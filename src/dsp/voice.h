#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED

#include "defines.h"
#include "mod_source.h"
#include "modulator.h"
#include "oscillator.h"
#include "patch.h"

#include <array>
#include <cstddef>
#include <random>
#include <vector>

namespace fmpire
{

class OscillatorVoice
{
public:
	OscillatorVoice();
	virtual ~OscillatorVoice() noexcept;

	// The oscillator of the current patch this voice plays.
	void set_oscillator(const Oscillator* const osc) { oscillator = osc; }

	void init(std::default_random_engine& rand,
			  const int note,
			  const float rate);

	// The output of this oscillator (one unison voice, before volume and pan,
	// but including the cross modulation it receives) that other oscillators
	// use as their modulator.
	inline float get_osc_value() const
	{
		return is_active() ? last_sample : 0.0f;
	}

	void calculate_unison_parameters();

	// What the other oscillators of the voice do to this one in one sample.
	struct CrossModulation
	{
		float gain = 1.0f;      // AM and RM
		float frequency = 1.0f; // FM: factor on the frequency
		float phase = 0.0f;     // PM: added to the phase, in cycles
	};

	// Once per block, before the routes are applied: finds the matrix cells
	// (type, modulator) that can be non-zero, i.e. those with a base depth or
	// a route, so an unused matrix costs nothing.
	void begin_block();
	void enable_cross_modulation(const size_t type, const size_t modulator);

	// Per sample: combines the outputs of the modulating oscillators.
	CrossModulation calculate_cross_modulation(
		const std::array<OscillatorVoice, FMPIRE_OSC_COUNT>& voices) const;

	void run_one_sample(float& left,
						float& right,
						const CrossModulation& cross);

	// Copies the base values (the knobs) from the oscillator, e.g. after they
	// changed while the note is playing.
	void refresh_base_values();

	inline bool is_active() const { return oscillator->params.active; }

	inline void clear_modulation()
	{
		mod_offsets.fill(0.0f);
		for (size_t entry = 0; entry < matrix_entry_count; entry++)
		{
			matrix_entries[entry].offset = 0.0f;
		}
	}

	inline void add_modulation(const TargetType target, const float value)
	{
		mod_offsets[static_cast<size_t>(target)] += value;
	}

	// What the routes added to `target` / a cross modulation depth in the
	// last sample.
	inline float get_modulation(const TargetType target) const
	{
		return mod_offsets[static_cast<size_t>(target)];
	}

	inline float get_cross_modulation(const size_t type,
									  const size_t modulator) const
	{
		const uint8_t entry = matrix_entry_index[type * FMPIRE_OSC_COUNT + modulator];
		return entry != no_entry ? matrix_entries[entry].offset : 0.0f;
	}

	// Modulation of the depth of `type` for modulator oscillator `modulator`;
	// only cells found by begin_block()/enable_cross_modulation() are used.
	inline void add_cross_modulation(const size_t type,
									 const size_t modulator,
									 const float value)
	{
		const uint8_t entry = matrix_entry_index[type * FMPIRE_OSC_COUNT + modulator];
		if (entry != no_entry)
		{
			matrix_entries[entry].offset += value;
		}
	}

private:
	const Oscillator* oscillator;
	float samplerate;
	size_t last_unison_size;
	float last_sample;

	std::array<float, FMPIRE_MAX_UNISON_AMOUNT> unison_phases;
	std::array<float, FMPIRE_MAX_UNISON_AMOUNT> unison_detunes;
	std::array<float, FMPIRE_MAX_UNISON_AMOUNT> unison_pans;
	size_t reference_index;


	float base_frequency;

	// Base values (the knobs). Modulation is added on top in run_one_sample.
	float volume;
	float wavetable_position;
	float detune;
	float pan;
	int note_shift;

	float unison_detune;
	float unison_spread;

	// indexed by the oscillator TargetTypes
	std::array<float, osc_target_count> mod_offsets;

	// The matrix cells that are in use this block, with the modulation their
	// routes added in the current sample.
	struct MatrixEntry
	{
		uint8_t type;
		uint8_t modulator;
		float offset;
	};

	static constexpr uint8_t no_entry = 255;

	std::array<MatrixEntry, matrix_type_count * FMPIRE_OSC_COUNT> matrix_entries;
	size_t matrix_entry_count;
	std::array<uint8_t, matrix_type_count * FMPIRE_OSC_COUNT> matrix_entry_index;
};

// Per-voice playback state of one Modulator (LFO or envelope).
class ModulatorVoice
{
public:
	ModulatorVoice();

	void init(const Modulator& mod, const float rate);

	// Advances one sample; updates get_value() / get_gain().
	void process(const Modulator& mod, const float bpm);

	// Note off: envelopes switch to their release section.
	void release(const Modulator& mod);

	// Curve value (0..1) and the output gain (the modulator's amount).
	inline float get_value() const { return value; }

	inline float get_gain() const { return gain; }

	// The generation of the modulator this state was initialised for.
	inline uint32_t get_generation() const { return generation; }

	inline void clear_modulation()
	{
		amount_offset = 0.0f;
		speed_offset = 0.0f;
	}

	void add_modulation(const TargetType target, const float value);

	// What the routes added to the amount / speed of the modulator.
	float get_modulation(const TargetType target) const;

	// Where the modulator currently is on its curve (0..1): the envelope
	// position or the LFO phase including its phase offset.
	float get_playhead(const Modulator& mod) const;

private:
	uint32_t generation;
	float samplerate;
	float release_decay;

	// envelope position or LFO phase, 0..1
	float position;
	bool released;

	float value;
	float gain;

	// smooths the jump when the release starts from the current level
	float release_offset;

	float amount_offset;
	float speed_offset;
};

class Voice
{
public:
	struct VoiceEndedCallback
	{
		virtual void on_voice_ended(Voice* const voice) = 0;
	};

	Voice(GlobalSources& global_sources, VoiceEndedCallback* ended_cb);
	Voice(const Voice& voice);
	virtual ~Voice() noexcept;

	void start(const size_t offset,
			   const int note_idx,
			   const float vol,
			   const float rate,
			   const float bpm);
	void stop(const size_t delay);
	void kill();

	void run(float** inout, size_t count, const float bpm);

	// Audio thread, before anything else in a block: the patch to play from
	// now on. Grows the per-modulator playback state if the patch has more
	// modulators than any patch before (the only allocation on the audio thread).
	void set_patch(const Patch& new_patch);

	inline void set_poly_pressure(const float pressure)
	{
		sources.poly_pressure = pressure;
	}

	bool is_active() const;
	int get_note() const;

	// True once the key was let go (the voice may still be playing its tail).
	bool is_stopping() const { return stop_voice; }

	// The offset the routes added to a target in the last sample (in knob
	// units, before clamping); for showing the modulation in the UI.
	float get_modulation_offset(const TargetType target,
								const size_t target_object) const;

	// The playhead of modulator `id` on its curve, or -1 if it isn't playing
	// in this voice.
	float get_playhead(const size_t id) const;

private:
	bool active;
	const Patch* patch;
	GlobalSources& globals;
	std::array<OscillatorVoice, FMPIRE_OSC_COUNT> oscillator_voices;
	std::vector<ModulatorVoice> modulator_voices;
	// scratch space of calculate_tail, one flag per modulator
	std::vector<uint8_t> tail_relevant;
	bool modulators_released;
	VoiceSources sources;
	float samplerate;
	std::default_random_engine rand;

	size_t delay;
	bool stop_voice;
	size_t stop_delay;
	// samples the voice keeps sounding after note off, and the length of the
	// fade-out at the end of that time
	size_t death_time;
	size_t fade_length;
	VoiceEndedCallback* const ended_callback;

	size_t age;
	int note;
	float volume;

	// How long the voice has to keep going after note off: the release of the
	// envelopes that shape the loudness, i.e. that (also through other
	// modulators) modulate the volume of an active oscillator. Envelopes that
	// only shape something else don't keep a note alive.
	size_t calculate_tail(const float bpm, const float rate);

	void apply_routes(const size_t mod_count, const size_t route_count);
	bool read_source(const SourceId& source,
					 const size_t mod_count,
					 float& value,
					 float& gain) const;
};

} // namespace fmpire

#endif // VOICE_H_INCLUDED
