#ifndef OSCILLATOR_H_INCLUDED
#define OSCILLATOR_H_INCLUDED

#include "wavetable.h"
#include "wavetable_creator.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace fmpire
{
struct OscillatorParams
{
	bool active = false;
	float volume = 0.1f;             // dynamic, mod
	float wavetable_position = 0.0f; // dynamic, mod
	float detune = 0.5f;             // dynamic, mod
	float pan = 0.5f;                // dynamic, mod
	int note_shift = 0;              // dynamic, no mod

	float phase_offset = 0.0f; // on voice start
	float phase_random = 0.0f; // on voice start

	uint32_t unison_size = 1;           // dynamic, no mod
	float unison_detune = 0.0f;         // dynamic, mod
	float unison_spread = 0.0f;         // dynamic, mod
	float unison_phase_random = 0.0f;   // on voice start
};

// An oscillator as the audio thread plays it (see Patch): the parameters and
// a wavetable that the state side prepared. Wavetables are shared between
// patches and only ever released by the state side.
struct Oscillator
{
	OscillatorParams params;
	std::shared_ptr<const Wavetable> wavetable;

	float sample(const float phase, const float wavetable_position) const
	{
		return wavetable ? wavetable->sample(wavetable_position, phase) : 0.0f;
	}
};

// The state side of an oscillator: parses and serializes the state strings and
// builds the wavetable from its waveforms. Never touched by the audio thread;
// what it produces goes into the Oscillators of a Patch.
class OscillatorState
{
public:
	OscillatorState();
	virtual ~OscillatorState() noexcept;

	void enable() { params.active = true; }

	void set_state(const std::string_view& key, std::string_view& state);

	std::string get_state() const;

	const OscillatorParams& get_params() const { return params; }

	// The current wavetable, for the audio thread's patch. Replaced by a new
	// one whenever the waveforms change, so it is never modified.
	std::shared_ptr<const Wavetable> get_wavetable() const { return wavetable; }

private:
	OscillatorParams params;
	std::shared_ptr<const Wavetable> wavetable;
	WavetableCreator wavetable_creator;

	void rebuild_wavetable();
};


} // namespace fmpire

#endif // OSCILLATOR_H_INCLUDED
