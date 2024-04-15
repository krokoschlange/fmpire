#include "wavetable_creator.h"

namespace fmpire
{

WavetableCreator::WavetableCreator()
{
}

WavetableCreator::~WavetableCreator() noexcept
{
}

float* WavetableCreator::create_wavetable(const size_t& width,
										  const size_t& height) const
{
	float* data = new float[width * height];
	for (size_t y = 0; y < height; y++)
	{
		const Waveform& waveform = waveforms[y];
		for (size_t x = 0; x < width; x++)
		{
			data[y * width + x] = waveform.sample(x);
		}
	}
	return data;
}

} // namespace fmpire
