#include "waveform.h"

#include "waveform_part.h"

namespace fmpire
{

Waveform::Waveform()
{
}

Waveform::~Waveform() noexcept
{
}

float Waveform::sample(const size_t position) const
{
	for (size_t part_idx = 0; part_idx < parts.size(); part_idx++)
	{
		const WaveformPart& part = parts[part_idx];
		if (part.contains(position))
		{
			return part.sample(position);
		}
	}
	return 0;
}


} // namespace fmpire
