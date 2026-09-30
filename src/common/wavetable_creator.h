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

	// Regenerates the content of all interpolated waveforms from their
	// neighbours. A run of consecutive interpolated waveforms is spread evenly
	// between the nearest normal waveform before and after it. A run without
	// an anchor on one side (at the table edge) just holds the other anchor;
	// the editor avoids that by calling bake_orphaned_interpolated().
	void update_interpolated();

	// For every run of interpolated waveforms that has no normal waveform on
	// one side (it reaches the table edge), turns the waveform at the edge into
	// a normal one that keeps its current content, so it becomes the new
	// anchor. Call it after removing or moving waveforms and before
	// update_interpolated(), as the content it keeps is what was generated for
	// the previous arrangement. Returns whether anything was changed.
	bool bake_orphaned_interpolated();

	bool is_interpolated(size_t index) const;

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

	// End (exclusive) of the run of interpolated waveforms starting at `start`.
	size_t find_interpolated_run_end(size_t start) const;

	// Regenerates waveforms [start, end), which are all interpolated.
	void update_interpolated_run(size_t start, size_t end);
};

} // namespace fmpire

#endif // WAVETABLE_CREATOR_H_INCLUDED
