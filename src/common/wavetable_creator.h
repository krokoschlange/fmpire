#ifndef WAVETABLE_CREATOR_H_INCLUDED
#define WAVETABLE_CREATOR_H_INCLUDED

#include "ref_counted.h"
#include "waveform.h"

#include <stddef.h>
#include <vector>

namespace fmpire
{

class WavetableCreator : public RefCounted
{
public:
	WavetableCreator();
	virtual ~WavetableCreator() noexcept;

	std::vector<float> create_wavetable() const;

	void set_state(const std::string& key, std::string_view& state);
	std::string get_state() const;

	Waveform* get_waveform(size_t index);

	void update();

	void get_size(uint32_t& w, uint32_t& h)
	{
		w = width;
		h = waveforms.size();
	}

	void add_waveform();
	void insert_waveform(uint32_t index, Waveform* const wf);

	void remove_waveform(uint32_t index);

private:
	uint32_t width;

	std::vector<Ref<Waveform>> waveforms;
};

} // namespace fmpire

#endif // WAVETABLE_CREATOR_H_INCLUDED
