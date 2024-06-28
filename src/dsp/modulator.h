#ifndef MODULATOR_H_INCLUDED
#define MODULATOR_H_INCLUDED

#include "defines.h"

#include <array>
#include <cstddef>
#include <vector>

namespace fmpire
{

class ModulatorTarget;
class ModulatorVoice;
class OscillatorVoice;
class Waveform;

class Modulator
{
public:
	Modulator();
	virtual ~Modulator() noexcept;

	float sample();

	float get_release();

private:
	bool global;
	bool sync;

	// lfo
	Waveform* waveform;
	float amount;    // dynamic, mod
	float frequency; // dynamic, mod
	float phase;     // on voice start

	// envelope
	float delay;   // on voice start
	float attack;  // on voice start
	float hold;    // on voice start
	float decay;   // on voice start
	float sustain; // on voice start
	float release; // on voice start

	std::vector<ModulatorTarget> targets;

	friend ModulatorTarget;
	friend ModulatorVoice;
};

struct ModulatorTargetObjects
{
	std::array<OscillatorVoice, FMPIRE_OSC_COUNT>& osc_voices;
	std::vector<ModulatorVoice>& mod_voices;
};

class ModulatorTarget
{
public:
	ModulatorTarget();
	virtual ~ModulatorTarget() noexcept;

	float* resolve(const ModulatorTargetObjects& objects) const;

private:
	float min;
	float max;

	enum class TargetType
	{
		OSC_VOLUME,
		OSC_WT_POS,
		OSC_DETUNE,
		OSC_PAN,
		OSC_UNISON_DETUNE,
		OSC_UNISON_SPREAD,
		MOD_AMOUNT,
		MOD_FREQ,
	};

	TargetType target_type;
	size_t target_object_id;

	friend ModulatorVoice;
};

} // namespace fmpire

#endif // MODULATOR_H_INCLUDED
