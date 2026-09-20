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

	float get_osc_value() const;

	void calculate_unison_parameters();

	void run_one_sample(float& left, float& right);

	// Copies the base values (the knobs) from the oscillator, e.g. after they
	// changed while the note is playing.
	void refresh_base_values();

	inline bool is_active() { return oscillator->params.active; }

	inline void clear_modulation() { mod_offsets.fill(0.0f); }

	inline void add_modulation(const TargetType target, const float value)
	{
		mod_offsets[static_cast<size_t>(target)] += value;
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

	bool is_active();
	int get_note() const;

private:
	bool active;
	const Patch* patch;
	GlobalSources& globals;
	std::array<OscillatorVoice, FMPIRE_OSC_COUNT> oscillator_voices;
	std::vector<ModulatorVoice> modulator_voices;
	bool modulators_released;
	VoiceSources sources;
	float samplerate;
	std::default_random_engine rand;

	size_t delay;
	bool stop_voice;
	size_t stop_delay;
	size_t death_time;
	VoiceEndedCallback* const ended_callback;

	size_t age;
	int note;
	float volume;

	void apply_routes(const size_t mod_count, const size_t route_count);
	bool read_source(const SourceId& source,
					 const size_t mod_count,
					 float& value,
					 float& gain) const;
};

} // namespace fmpire

#endif // VOICE_H_INCLUDED
