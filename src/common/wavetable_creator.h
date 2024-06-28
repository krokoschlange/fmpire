#ifndef WAVETABLE_CREATOR_H_INCLUDED
#define WAVETABLE_CREATOR_H_INCLUDED

#include "extra/ScopedPointer.hpp"
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

	ScopedPointer<std::vector<float>> create_wavetable(
		const size_t width) const;

	void set_state(const std::string& key, std::string_view& state);
	std::string get_state() const;

private:
	std::vector<Waveform> waveforms;
};

} // namespace fmpire

#endif // WAVETABLE_CREATOR_H_INCLUDED
