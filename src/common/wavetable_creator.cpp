#include "wavetable_creator.h"

#include "defines.h"
#include "utils.h"
#include "waveform.h"

#include <string>

namespace fmpire
{

WavetableCreator::WavetableCreator() :
	width(512)
{
	waveforms = {new Waveform()};
}

WavetableCreator::~WavetableCreator() noexcept
{
}

std::vector<float> WavetableCreator::create_wavetable() const
{
	const size_t height = waveforms.size();
	std::vector<float> data(width * height, 0);
	for (size_t y = 0; y < height; y++)
	{
		const Waveform& waveform = *waveforms[y];
		for (size_t x = 0; x < width; x++)
		{
			data[y * width + x] = waveform.sample(x);
		}
	}
	return data;
}

void WavetableCreator::set_state(const std::string& key,
								 std::string_view& state)
{
	if (key == KEY_WT_ALL)
	{
		uint32_t amount = 0;
		decode_base64(state,
					  reinterpret_cast<uint8_t*>(&amount),
					  sizeof(amount));
		waveforms.resize(amount);

		for (size_t i = 0; i < waveforms.size(); i++)
		{
			if (waveforms[i] == nullptr)
			{
				waveforms[i] = new Waveform();
			}
		}

		for (size_t wf_idx = 0; wf_idx < waveforms.size(); wf_idx++)
		{
			waveforms[wf_idx]->set_state(state);
		}
	}
	else if (key == KEY_WT_INSERT)
	{
		uint32_t index = 0;
		decode_base64(state, reinterpret_cast<uint8_t*>(&index), sizeof(index));
		if (index > waveforms.size())
		{
			waveforms.reserve(index);

			while (waveforms.size() < index)
			{
				waveforms.push_back(new Waveform());
			}
		}

		waveforms.insert(waveforms.begin() + index, new Waveform());
		waveforms[index]->set_state(state);
	}
	else if (key == KEY_WT_UPDATE)
	{
		uint32_t index = 0;
		decode_base64(state, reinterpret_cast<uint8_t*>(&index), sizeof(index));
		if (index >= waveforms.size())
		{
			waveforms.reserve(index + 1);

			while (waveforms.size() <= index)
			{
				waveforms.push_back(new Waveform());
			}
		}

		waveforms[index]->set_state(state);
	}
	else if (key == KEY_WT_REMOVE)
	{
		uint32_t index = 0;
		decode_base64(state, reinterpret_cast<uint8_t*>(&index), sizeof(index));
		if (index < waveforms.size())
		{
			waveforms.erase(waveforms.begin() + index);
		}
	}

	update();
}

std::string WavetableCreator::get_state() const
{
	std::string data;
	uint32_t amount = waveforms.size();
	data += encode_base64(reinterpret_cast<const uint8_t*>(&amount),
						  sizeof(amount));
	for (size_t wf_idx = 0; wf_idx < waveforms.size(); wf_idx++)
	{
		data += waveforms[wf_idx]->get_state();
	}

	return data;
}

Waveform* WavetableCreator::get_waveform(size_t index)
{
	if (index >= waveforms.size())
	{
		return nullptr;
	}
	return waveforms[index];
}

void WavetableCreator::update()
{
	for (size_t i = 0; i < waveforms.size(); i++)
	{
		waveforms[i]->update(width, i);
	}
}

void WavetableCreator::add_waveform()
{
	waveforms.push_back(new Waveform());
	waveforms.back()->update(width, waveforms.size() - 1);
}

void WavetableCreator::insert_waveform(uint32_t index, Waveform* const wf)
{
	if (index >= waveforms.size())
	{
		waveforms.push_back(wf);
		return;
	}
	waveforms.insert(waveforms.begin() + index, wf);
}

void WavetableCreator::remove_waveform(uint32_t index)
{
	if (index >= waveforms.size())
	{
		return;
	}

	waveforms.erase(waveforms.begin() + index);
}

} // namespace fmpire
