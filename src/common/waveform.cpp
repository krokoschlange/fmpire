#include "waveform.h"

#include "utils.h"
#include "waveform_part.h"

#define WAVEFORM_STRING std::string("WAVEFORM")

namespace fmpire
{

Waveform::Waveform() :
	width(512)
{
}

Waveform::~Waveform() noexcept
{
}

float Waveform::sample(const size_t position) const
{
	for (size_t part_idx = 0; part_idx < parts.size(); part_idx++)
	{
		const WaveformPart& part = *parts[part_idx];
		if (part.contains(position))
		{
			return part.sample(position);
		}
	}
	return 0;
}

std::vector<float> Waveform::sample_all() const
{
	std::vector<float> samples(width, 0);

	for (size_t smpl = 0; smpl < width; smpl++)
	{
		samples[smpl] = sample(smpl);
	}

	return samples;
}

WaveformPart* Waveform::get_part(size_t position)
{
	for (size_t part_idx = 0; part_idx < parts.size(); part_idx++)
	{
		WaveformPart& part = *parts[part_idx];
		if (part.contains(position))
		{
			return &part;
		}
	}
	return nullptr;
}

void Waveform::set_state(std::string_view& state)
{
	if (!state.starts_with(WAVEFORM_STRING))
	{
		return;
	}
	state.remove_prefix(WAVEFORM_STRING.size());
	decode_base32(state, reinterpret_cast<uint8_t*>(width), sizeof(width));
	uint32_t parts_count = 0;
	decode_base32(state,
				  reinterpret_cast<uint8_t*>(parts_count),
				  sizeof(parts_count));

	parts.resize(parts_count);
	for (size_t part_idx = 0; part_idx < parts.size(); part_idx++)
	{
		uint32_t part_size = 0;
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(part_size),
					  sizeof(part_size));
		std::string_view part_str = state.substr(0, part_size);

		if (part_size > state.size())
		{
			return;
		}
		state.remove_prefix(part_size);

		parts[part_idx] = WaveformPart::create(part_str);
	}
}

std::string Waveform::get_state() const
{
	std::string data(WAVEFORM_STRING);
	data +=
		encode_base32(reinterpret_cast<const uint8_t*>(width), sizeof(width));
	uint32_t parts_count = parts.size();
	data += encode_base32(reinterpret_cast<const uint8_t*>(parts_count),
						  sizeof(parts_count));

	for (size_t part_idx = 0; part_idx < parts.size(); part_idx++)
	{
		std::string part_data = parts[part_idx]->encode();
		uint32_t part_size = part_data.size();
		data += encode_base32(reinterpret_cast<const uint8_t*>(part_size),
							  sizeof(part_size));
		data += part_data;
	}

	return data;
}


} // namespace fmpire
