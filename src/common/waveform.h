#ifndef WAVEFORM_H_INCLUDED
#define WAVEFORM_H_INCLUDED

#include <memory>
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

private:
	size_t width;
	std::vector<WaveformPart> parts;
};

} // namespace fmpire

#endif // WAVEFORM_H_INCLUDED
