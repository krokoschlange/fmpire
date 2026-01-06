#ifndef WAVEFORM_H_INCLUDED
#define WAVEFORM_H_INCLUDED

#include "ref_counted.h"
#include "waveform_tools.h"
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace fmpire
{

class WaveformPart;

class Waveform : public RefCounted
{
public:
	Waveform();
	virtual ~Waveform() noexcept;

	float sample(const size_t position) const;

	std::vector<float> sample_all() const;

	WaveformPart* get_part(size_t position);
	WaveformPart* get_part_by_idx(size_t idx);

	void set_state(std::string_view& state);
	std::string get_state() const;


	void insert_part(WaveformPart* part);

	void update(uint32_t w, uint32_t idx);

	void remove_part(size_t index);

	void remove_part(const Ref<WaveformPart>& part);

	inline uint32_t get_width() const { return width; }

	inline uint32_t get_index() const { return index; }

private:
	uint32_t width;
	uint32_t index;
	std::vector<Ref<WaveformPart>> parts;
};

} // namespace fmpire

#endif // WAVEFORM_H_INCLUDED
