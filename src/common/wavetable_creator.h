#ifndef WAVETABLE_CREATOR_H_INCLUDED
#define WAVETABLE_CREATOR_H_INCLUDED

#include "waveform.h"

#include <stddef.h>
#include <vector>

namespace fmpire
{

class WavetableCreator
{
public:
	WavetableCreator();
	virtual ~WavetableCreator() noexcept;

	float* create_wavetable(const size_t& width, const size_t& height) const;

private:
	std::vector<Waveform> waveforms;
};

} // namespace fmpire

#endif // WAVETABLE_CREATOR_H_INCLUDED
