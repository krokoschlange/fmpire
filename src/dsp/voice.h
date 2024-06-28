#ifndef VOICE_H_INCLUDED
#define VOICE_H_INCLUDED

#include "defines.h"
#include "oscillator.h"

#include <array>
#include <cstddef>
#include <memory>
#include <random>

namespace fmpire
{
class Modulator;
class ModulatorTarget;
struct ModulatorTargetObjects;

class OscillatorVoice
{
public:
	OscillatorVoice(Oscillator& osc);
	virtual ~OscillatorVoice() noexcept;

	void init(std::default_random_engine& rand,
			  const int note,
			  const float rate);

	float get_osc_value() const;

	void calculate_unison_parameters();

	void run_one_sample(float& left, float& right);

	void set_state(const std::string& key, std::string_view& state);

	inline bool is_active() { return oscillator->active; }


private:
	Oscillator* const oscillator;
	float samplerate;
	size_t last_unison_size;
	float last_sample;

	std::array<float, FMPIRE_MAX_UNISON_AMOUNT> unison_phases;
	std::array<float, FMPIRE_MAX_UNISON_AMOUNT> unison_detunes;
	std::array<float, FMPIRE_MAX_UNISON_AMOUNT> unison_pans;
	size_t reference_index;


	float base_frequency;

	float volume;
	float wavetable_position;
	float detune;
	float pan;
	int note_shift;

	float unison_detune;
	float unison_spread;

	friend ModulatorTarget;
};

class ModulatorVoice
{
public:
	ModulatorVoice();
	virtual ~ModulatorVoice() noexcept;

	void init(std::shared_ptr<Modulator> mod,
			  const ModulatorTargetObjects& target_objects,
			  const float rate);

	void modulate();

	void release();

	void set_state(const std::string& key, std::string_view& state);

private:
	std::shared_ptr<Modulator> modulator;
	std::vector<float*> targets;

	float samplerate;

	float amount;
	float frequency;
	float phase;

	bool released;
	size_t frames_since_start;
	size_t frames_since_release;

	friend ModulatorTarget;
};

class Voice
{
public:
	struct VoiceEndedCallback
	{
		virtual void on_voice_ended(Voice* const voice) = 0;
	};

	Voice(std::array<Oscillator, FMPIRE_OSC_COUNT>& oscs,
		  std::vector<std::shared_ptr<Modulator>>& mods,
		  VoiceEndedCallback* ended_cb);
	Voice(const Voice& voice);
	virtual ~Voice() noexcept;

	void start(const size_t offset,
			   const int note_idx,
			   const float vol,
			   const float rate);
	void stop(const size_t delay);
	void kill();

	void run(float** inout, size_t count);

	void set_state(std::string_view& key, std::string_view& state);

	bool is_active();
	int get_note() const;

private:
	bool active;
	std::array<Oscillator, FMPIRE_OSC_COUNT>& oscillators;
	std::vector<std::shared_ptr<Modulator>>& modulators;
	std::array<OscillatorVoice, FMPIRE_OSC_COUNT> oscillator_voices;
	std::vector<ModulatorVoice> modulator_voices;
	std::default_random_engine rand;

	size_t delay;
	bool stop_voice;
	size_t stop_delay;
	size_t death_time;
	VoiceEndedCallback* const ended_callback;

	size_t age;
	int note;
	float volume;
};

} // namespace fmpire

#endif // VOICE_H_INCLUDED
