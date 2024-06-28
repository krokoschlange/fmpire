#include "wavetable.h"

#include "utils.h"
#include <cmath>

namespace fmpire
{

Wavetable::Wavetable() :
	data(nullptr),
	width(0),
	height(0)
{
}

Wavetable::~Wavetable() noexcept
{
}

float Wavetable::sample(const float position,
						const float phase,
						const bool interp_waves,
						const bool interp_phase) const
{
	if (!data)
	{
		return 0;
	}

	size_t pos_smpl, phase_smpl;
	get_sample_position(position, phase, pos_smpl, phase_smpl);
	float sample = 0;
	if (interp_waves && interp_phase)
	{
		float pos_lerp = get_position_interpolation_factor(position);
		float phase_lerp = get_phase_interpolation_factor(phase);
		float sample00 = read(pos_smpl, phase_smpl);
		float sample01 = read(pos_smpl, phase_smpl + 1);
		float sample10 = read(pos_smpl + 1, phase_smpl);
		float sample11 = read(pos_smpl + 1, phase_smpl + 1);
		float sample0 = lerp(sample00, sample01, phase_lerp);
		float sample1 = lerp(sample10, sample11, phase_lerp);
		sample = lerp(sample0, sample1, pos_lerp);
	}
	else if (interp_waves)
	{
		float pos_lerp = get_position_interpolation_factor(position);
		float sample00 = read(pos_smpl, phase_smpl);
		float sample10 = read(pos_smpl + 1, phase_smpl);
		sample = lerp(sample00, sample10, pos_lerp);
	}
	else if (interp_phase)
	{
		float phase_lerp = get_phase_interpolation_factor(phase);
		float sample00 = read(pos_smpl, phase_smpl);
		float sample01 = read(pos_smpl, phase_smpl + 1);
		sample = lerp(sample00, sample01, phase_lerp);
	}
	else
	{
		sample = read(pos_smpl, phase_smpl);
	}
	return sample;
}

void Wavetable::get_size(size_t& w, size_t& h) const
{
	w = width;
	h = height;
}

void Wavetable::get_sample_position(const float position,
									const float phase,
									size_t& pos_smpl,
									size_t& phase_smpl) const
{
	phase_smpl = (size_t) (phase * width) % width;
	pos_smpl = (size_t) (position * (height - 1));
}

float Wavetable::get_position_interpolation_factor(const float position) const
{
	float whole;
	return std::modf(position * height, &whole);
}

float Wavetable::get_phase_interpolation_factor(const float phase) const
{
	float whole;
	return std::modf(phase * width, &whole);
}

float Wavetable::read(const size_t position, const size_t phase) const
{
	size_t wrapped_phase = phase % width;
	size_t pos = std::min(position, height - 1);
	size_t index = pos * width + wrapped_phase;
	return (*data)[index];
}


} // namespace fmpire


