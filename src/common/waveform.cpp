#include "waveform.h"

#include "ref_counted.h"
#include "utils.h"
#include "waveform_part.h"

#include <cstddef>

#define WAVEFORM_STRING std::string("WAVEFORM")

namespace fmpire
{

Waveform::Waveform() :
	width(512),
	index(0)
{
	Ref<FunctionWaveformPart> default_sine = new FunctionWaveformPart();
	default_sine->set_function("sin(2*pi*x)");
	default_sine->set_start(0);
	default_sine->set_end(width);
	parts.push_back(static_ref_cast<WaveformPart>(default_sine));

	update(width, 0);
}

Waveform::~Waveform() noexcept
{
}

float Waveform::sample(const size_t position) const
{
	for (size_t part_idx = parts.size(); part_idx > 0; part_idx--)
	{
		const WaveformPart& part = *parts[part_idx - 1];
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
	for (size_t part_idx = parts.size(); part_idx > 0; part_idx--)
	{
		WaveformPart* part = parts[part_idx - 1];
		if (part->contains(position))
		{
			return part;
		}
	}
	return nullptr;
}

WaveformPart* Waveform::get_part_by_idx(size_t idx)
{
	if (idx >= parts.size())
	{
		return nullptr;
	}

	return parts[idx];
}

void Waveform::set_state(std::string_view& state)
{
	if (!state.starts_with(WAVEFORM_STRING))
	{
		return;
	}
	state.remove_prefix(WAVEFORM_STRING.size());
	decode_base64(state, reinterpret_cast<uint8_t*>(&width), sizeof(width));
	uint32_t parts_count = 0;
	decode_base64(state,
				  reinterpret_cast<uint8_t*>(&parts_count),
				  sizeof(parts_count));

	parts.resize(parts_count);
	for (size_t part_idx = 0; part_idx < parts.size(); part_idx++)
	{
		uint32_t part_size = 0;
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&part_size),
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
		encode_base64(reinterpret_cast<const uint8_t*>(&width), sizeof(width));
	uint32_t parts_count = parts.size();
	data += encode_base64(reinterpret_cast<const uint8_t*>(&parts_count),
						  sizeof(parts_count));

	for (size_t part_idx = 0; part_idx < parts.size(); part_idx++)
	{
		std::string part_data = parts[part_idx]->encode();
		uint32_t part_size = part_data.size();
		data += encode_base64(reinterpret_cast<const uint8_t*>(&part_size),
							  sizeof(part_size));
		data += part_data;
	}

	return data;
}

void Waveform::insert_part(WaveformPart* part)
{
	part->set_width_and_index(width, parts.size());
	parts.push_back(part);
}

void Waveform::update(uint32_t w, uint32_t idx)
{
	width = w;
	index = idx;

	for (size_t i = 0; i < parts.size(); i++)
	{
		parts[i]->set_width_and_index(width, idx);
	}
}

void Waveform::remove_part(size_t idx)
{
	if (idx >= parts.size())
	{
		return;
	}

	parts.erase(parts.begin() + idx);
}

void Waveform::remove_part(const Ref<WaveformPart>& part)
{
	for (size_t i = 0; i < parts.size(); i++)
	{
		if (parts[i] == part)
		{
			parts.erase(parts.begin() + i);
			return;
		}
	}
}


} // namespace fmpire
