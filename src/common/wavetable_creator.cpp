#include "wavetable_creator.h"

#include "defines.h"
#include "utils.h"
#include "waveform_part.h"

#include <string>

namespace fmpire
{

WavetableCreator::WavetableCreator()
{
}

WavetableCreator::~WavetableCreator() noexcept
{
}

ScopedPointer<std::vector<float>> WavetableCreator::create_wavetable(
	const size_t width) const
{
	const size_t height = waveforms.size();
	std::vector<float>* data = new std::vector<float>(width * height, 0);
	for (size_t y = 0; y < height; y++)
	{
		const Waveform& waveform = waveforms[y];
		for (size_t x = 0; x < width; x++)
		{
			(*data)[y * width + x] = waveform.sample(x);
		}
	}
	return ScopedPointer<std::vector<float>>(data);
}

void WavetableCreator::set_state(const std::string& key,
								 std::string_view& state)
{
	if (key == KEY_WT_ALL)
	{
		uint32_t amount = 0;
		decode_base32(state,
					  reinterpret_cast<uint8_t*>(&amount),
					  sizeof(amount));
		waveforms.resize(amount);
		for (size_t wf_idx = 0; wf_idx < waveforms.size(); wf_idx++)
		{
			waveforms[wf_idx].set_state(state);
		}
	}
	else if (key == KEY_WT_UPDATE)
	{
		uint32_t index = 0;
		decode_base32(state, reinterpret_cast<uint8_t*>(&index), sizeof(index));
		if (index >= waveforms.size())
		{
			waveforms.resize(index + 1);
		}

		waveforms[index].set_state(state);
	}
	else if (key == KEY_WT_REMOVE)
	{
		uint32_t index = 0;
		decode_base32(state, reinterpret_cast<uint8_t*>(&index), sizeof(index));
		if (index < waveforms.size())
		{
			waveforms.erase(waveforms.begin() + index);
		}
	}
}

std::string WavetableCreator::get_state() const
{
	std::string data;
	uint32_t amount = waveforms.size();
	data += encode_base32(reinterpret_cast<const uint8_t*>(&amount),
						  sizeof(amount));
	for (size_t wf_idx = 0; wf_idx < waveforms.size(); wf_idx++)
	{
		data += waveforms[wf_idx].get_state();
	}

	return data;
}

} // namespace fmpire
