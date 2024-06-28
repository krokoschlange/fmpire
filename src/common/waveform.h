#ifndef WAVEFORM_H_INCLUDED
#define WAVEFORM_H_INCLUDED

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace fmpire
{

class WaveformPart;

class Waveform
{
public:
	Waveform();
	virtual ~Waveform() noexcept;

	float sample(const size_t position) const;

	std::vector<float> sample_all() const;

	WaveformPart* get_part(size_t position);

	void set_state(std::string_view& state);
	std::string get_state() const;

private:
	uint32_t width;
	std::vector<std::shared_ptr<WaveformPart>> parts;
};

} // namespace fmpire

#endif // WAVEFORM_H_INCLUDED
