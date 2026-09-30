#include "wavetable_creator.h"

#include "defines.h"
#include "utils.h"
#include "waveform.h"
#include "waveform_part.h"

#include <algorithm>
#include <string>

namespace fmpire
{

namespace
{

using Harmonic = HarmonicsWaveformPart::Harmonic;

bool is_spectral(const InterpolationType type)
{
	return type != InterpolationType::NONE
		&& type != InterpolationType::CROSSFADE;
}

void zero_phases(std::vector<Harmonic>& harmonics, const InterpolationType type)
{
	if (type == InterpolationType::SPECTRAL_ZERO_ALL)
	{
		for (Harmonic& harmonic : harmonics)
		{
			harmonic.phase = 0.0f;
		}
	}
	else if (type == InterpolationType::SPECTRAL_ZERO_FUNDAMENTAL
			 && harmonics.size() > 1)
	{
		harmonics[1].phase = 0.0f;
	}
}

Ref<WaveformPart> crossfade(const std::vector<float>& before,
							const std::vector<float>& after,
							const float t)
{
	Ref<SamplesWaveformPart> part = new SamplesWaveformPart();
	std::vector<float>& mixed = part->get_samples();
	mixed.resize(before.size());
	for (size_t smpl = 0; smpl < mixed.size(); smpl++)
	{
		mixed[smpl] = lerp(before[smpl], after[smpl], t);
	}
	return static_ref_cast<WaveformPart>(part);
}

Ref<WaveformPart> spectral_morph(std::vector<Harmonic> before,
								 std::vector<Harmonic> after,
								 const InterpolationType type,
								 const float t)
{
	const size_t harmonic_count = std::max(before.size(), after.size());
	before.resize(harmonic_count, {0.0f, 0.0f});
	after.resize(harmonic_count, {0.0f, 0.0f});
	zero_phases(before, type);
	zero_phases(after, type);

	Ref<HarmonicsWaveformPart> part = new HarmonicsWaveformPart();
	std::vector<Harmonic>& mixed = part->get_harmonics();
	mixed.resize(harmonic_count);
	for (size_t harm = 0; harm < harmonic_count; harm++)
	{
		mixed[harm].amplitude =
			lerp(before[harm].amplitude, after[harm].amplitude, t);
		mixed[harm].phase = lerp(before[harm].phase, after[harm].phase, t);
	}
	return static_ref_cast<WaveformPart>(part);
}

} // namespace

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
	update_interpolated();
}

void WavetableCreator::update_interpolated()
{
	size_t run_start = 0;
	while (run_start < waveforms.size())
	{
		if (!waveforms[run_start]->is_interpolated())
		{
			run_start++;
			continue;
		}

		const size_t run_end = find_interpolated_run_end(run_start);
		update_interpolated_run(run_start, run_end);
		run_start = run_end;
	}
}

bool WavetableCreator::bake_orphaned_interpolated()
{
	bool baked = false;

	size_t run_start = 0;
	while (run_start < waveforms.size())
	{
		if (!waveforms[run_start]->is_interpolated())
		{
			run_start++;
			continue;
		}

		// Only the waveform at the table edge is made a normal one (its
		// generated part stays and becomes its own content). It is the new
		// anchor, the rest of the run is interpolated between it and the
		// anchor on the other side.
		const size_t run_end = find_interpolated_run_end(run_start);
		if (run_start == 0)
		{
			waveforms[run_start]->set_interpolation(InterpolationType::NONE);
			baked = true;
		}
		if (run_end == waveforms.size())
		{
			waveforms[run_end - 1]->set_interpolation(InterpolationType::NONE);
			baked = true;
		}
		run_start = run_end;
	}

	return baked;
}

size_t WavetableCreator::find_interpolated_run_end(const size_t start) const
{
	size_t end = start;
	while (end < waveforms.size() && waveforms[end]->is_interpolated())
	{
		end++;
	}
	return end;
}

void WavetableCreator::update_interpolated_run(const size_t start,
											   const size_t end)
{
	// Both neighbours of a maximal run are normal waveforms (or missing).
	Waveform* before = start > 0 ? get_waveform(start - 1) : nullptr;
	Waveform* after = get_waveform(end);
	if (!before)
	{
		before = after;
	}
	if (!after)
	{
		after = before;
	}

	std::vector<float> samples_before(width, 0.0f);
	std::vector<float> samples_after(width, 0.0f);
	if (before)
	{
		samples_before = before->sample_all();
		samples_before.resize(width, 0.0f);
		samples_after = after->sample_all();
		samples_after.resize(width, 0.0f);
	}

	std::vector<Harmonic> harmonics_before;
	std::vector<Harmonic> harmonics_after;
	bool analyzed = false;

	const size_t count = end - start;
	for (size_t i = 0; i < count; i++)
	{
		Waveform& waveform = *waveforms[start + i];
		const InterpolationType type = waveform.get_interpolation();
		const float t = (float) (i + 1) / (count + 1);

		if (is_spectral(type))
		{
			if (!analyzed)
			{
				harmonics_before = analyze_harmonics(samples_before, true);
				harmonics_after = analyze_harmonics(samples_after, true);
				analyzed = true;
			}
			waveform.set_generated_part(
				spectral_morph(harmonics_before, harmonics_after, type, t));
		}
		else
		{
			waveform.set_generated_part(
				crossfade(samples_before, samples_after, t));
		}
	}
}

bool WavetableCreator::is_interpolated(const size_t index) const
{
	return index < waveforms.size() && waveforms[index]->is_interpolated();
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
