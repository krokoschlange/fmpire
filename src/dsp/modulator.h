#ifndef MODULATOR_H_INCLUDED
#define MODULATOR_H_INCLUDED

#include "curve.h"
#include "mod_types.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace fmpire
{

// One connection from a source to a target parameter. The target parameter's
// own value stays the base; every route adds
//   amount * (bipolar ? 2 * source - 1 : source)
// on top, in the parameter's normalized 0..1 range.
struct ModRoute
{
	bool active = false;
	float amount = 0.0f;
	bool bipolar = false;

	SourceId source;
	TargetType target = TargetType::OSC_VOLUME;
	uint16_t target_object = 0;
};

// A user-drawn LFO or envelope, as the audio thread plays it: the settings and
// the baked curve table (shared by all voices; the per-voice playback state is
// in ModulatorVoice). Plain data, so it can be built on a state thread and
// copied into place without allocating.
class Modulator
{
public:
	using Type = ModulatorType;

	Modulator();
	Modulator(const ModulatorSettings& settings, const Curve& curve);

	void set_enabled(const bool value) { enabled = value; }

	// Bumped by whoever creates the modulator, so voices can tell a modulator
	// that was replaced (or created after the note started) from one that only
	// had its settings changed.
	void set_generation(const uint32_t value) { generation = value; }

	uint32_t get_generation() const { return generation; }

	// out-of-range settings are clamped
	ModulatorSettings get_settings() const;

	bool is_enabled() const { return enabled; }

	Type get_type() const { return type; }

	bool uses_beats() const { return use_beats; }

	float get_amount() const { return amount; }

	float get_phase_offset() const { return phase_offset; }

	float get_sustain_pos() const { return sustain_pos; }

	const float* get_table() const { return table.data(); }

	// One full pass through the curve, in seconds.
	float get_length_seconds(const float bpm) const;

	// Longest possible time an envelope keeps playing after note off.
	float get_release_seconds(const float bpm) const;

private:
	bool enabled;
	uint32_t generation;
	Type type;
	bool use_beats;
	float length_seconds;
	float length_beats;
	float amount;
	float phase_offset;
	float sustain_pos;

	std::array<float, Curve::table_size + 1> table;
};

} // namespace fmpire

#endif // MODULATOR_H_INCLUDED
