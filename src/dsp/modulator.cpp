#include "modulator.h"

#include <algorithm>
#include <cmath>

namespace fmpire
{

// A disabled placeholder; its table is never read.
Modulator::Modulator() :
	enabled(false),
	generation(0),
	type(Type::ENVELOPE),
	use_beats(false),
	length_seconds(1.0f),
	length_beats(1.0f),
	amount(1.0f),
	phase_offset(0.0f),
	sustain_pos(0.0f)
{
	table.fill(0.0f);
}

Modulator::Modulator(const ModulatorSettings& settings, const Curve& curve) :
	enabled(false),
	generation(0),
	type(settings.type),
	use_beats(settings.use_beats),
	length_seconds(std::max(settings.length_seconds, 0.001f)),
	length_beats(std::max(settings.length_beats, 0.001f)),
	amount(std::clamp(settings.amount, 0.0f, 1.0f)),
	phase_offset(settings.phase_offset - std::floor(settings.phase_offset)),
	sustain_pos(curve.point(curve.get_sustain_index()).x)
{
	curve.bake(table.data());
}

ModulatorSettings Modulator::get_settings() const
{
	ModulatorSettings settings;
	settings.type = type;
	settings.use_beats = use_beats;
	settings.length_seconds = length_seconds;
	settings.length_beats = length_beats;
	settings.amount = amount;
	settings.phase_offset = phase_offset;
	return settings;
}

float Modulator::get_length_seconds(const float bpm) const
{
	const float seconds = use_beats ? length_beats * 60.0f / std::max(bpm, 1.0f)
									: length_seconds;
	return std::max(seconds, 0.001f);
}

float Modulator::get_release_seconds(const float bpm) const
{
	if (type != Type::ENVELOPE)
	{
		return 0.0f;
	}
	return (1.0f - sustain_pos) * get_length_seconds(bpm);
}

} // namespace fmpire
