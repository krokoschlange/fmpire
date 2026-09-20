#ifndef MOD_TYPES_H_INCLUDED
#define MOD_TYPES_H_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace fmpire
{

// Everything that can drive a modulation route. LFO/ENV modulators, host
// macros and MIDI all look the same to routing: a SourceId that resolves to a
// value between 0 and 1.
enum class SourceType : uint8_t
{
	NONE,
	MODULATOR, // LFO/ENV, index = modulator id
	MACRO,     // host-automatable parameter, index = macro number
	MIDI_CC,   // index = controller number
	PITCH_BEND,
	CHANNEL_PRESSURE,
	POLY_PRESSURE,
	VELOCITY,
	KEY,
	COUNT,
};

struct SourceId
{
	SourceType type = SourceType::NONE;
	uint16_t index = 0;

	inline uint32_t pack() const
	{
		return (static_cast<uint32_t>(type) << 16) | index;
	}

	static inline SourceId unpack(const uint32_t packed)
	{
		SourceId id;
		const uint32_t raw_type = packed >> 16;
		id.type = raw_type < static_cast<uint32_t>(SourceType::COUNT)
					? static_cast<SourceType>(raw_type)
					: SourceType::NONE;
		id.index = packed & 0xffff;
		return id;
	}

	bool operator==(const SourceId& other) const = default;
};

enum class TargetType : uint8_t
{
	OSC_VOLUME,
	OSC_WT_POS,
	OSC_DETUNE,
	OSC_PAN,
	OSC_UNISON_DETUNE,
	OSC_UNISON_SPREAD,
	MOD_AMOUNT,
	MOD_FREQ,
	COUNT,
};

// The first osc_target_count target types belong to an oscillator, the rest to
// a modulator.
constexpr size_t osc_target_count = 6;

enum class ModulatorType : uint8_t
{
	ENVELOPE,
	LFO,
};

// The scalar settings of an LFO/envelope (the curve is stored separately).
// Shared by the UI and the DSP so the state format is defined once.
struct ModulatorSettings
{
	ModulatorType type = ModulatorType::ENVELOPE;
	bool use_beats = false;
	float length_seconds = 1.0f;
	float length_beats = 1.0f; // in quarter notes
	float amount = 1.0f;
	float phase_offset = 0.0f;

	std::string encode() const;
	void decode(std::string_view& data);
};

// One connection from a source to a target parameter. `slot` is chosen by the
// UI and honoured by the DSP, so both sides always agree on route identity.
struct RouteSettings
{
	uint32_t slot = 0;
	SourceId source;
	TargetType target = TargetType::OSC_VOLUME;
	uint16_t target_object = 0;
	float amount = 0.0f;
	bool bipolar = false;

	std::string encode() const;
	void decode(std::string_view& data);
};

} // namespace fmpire

#endif // MOD_TYPES_H_INCLUDED
