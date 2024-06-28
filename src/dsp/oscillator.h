#ifndef OSCILLATOR_H_INCLUDED
#define OSCILLATOR_H_INCLUDED

#include "wavetable.h"
#include "wavetable_creator.h"

namespace fmpire
{
class ModulatorTarget;
class OscillatorVoice;

class Oscillator
{
public:
	Oscillator();
	virtual ~Oscillator() noexcept;

	float sample(const float phase, const float wavetable_position) const;

	void enable() { active = true; }

	void set_state(const std::string& key, std::string_view& state);

	std::string get_state() const;

private:
	Wavetable wavetable;
	WavetableCreator wavetable_creator;
	bool active;
	float volume;             // dynamic, mod
	float wavetable_position; // dynamic, mod
	float detune;             // dynamic, mod
	float pan;                // dynamic, mod
	int note_shift;           // dynamic, no mod

	float phase_offset; // on voice start
	float phase_random; // on voice start

	uint32_t unison_size;      // dynamic, no mod
	float unison_detune;       // dynamic, mod
	float unison_spread;       // dynamic, mod
	float unison_phase_random; // on voice start


	friend OscillatorVoice;
};


} // namespace fmpire

#endif // OSCILLATOR_H_INCLUDED
