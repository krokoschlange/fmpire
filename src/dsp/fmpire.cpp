
#include "fmpire.hpp"

#include <string>

namespace fmpire
{

FMpire::FMpire() :
	Plugin(0, 0, 0),
	sync_time(false),
	self_frame(0),
	volume(1)
{
}

FMpire::~FMpire() noexcept
{
}

const char* FMpire::getLabel() const
{
	return "FMpire";
}

const char* FMpire::getDescription() const
{
	return "FMpire";
}

const char* FMpire::getMaker() const
{
	return "krokoschlange";
}

const char* FMpire::getHomePage() const
{
	return "github.com/krokoschlange/fmpire";
}

const char* FMpire::getLicense() const
{
	return "MIT";
}

uint32_t FMpire::getVersion() const
{
	return d_version(0, 1, 0);
}

int64_t FMpire::getUniqueId() const
{
	return d_cconst('K', 'f', 'm', 'p');
}

String FMpire::getState(const char* key) const
{
	return String();
}

void FMpire::setState(const char* key, const char* value)
{
	d_stdout("state set (dsp) %s %s", key, value);
	if (std::string(key) == "vol")
	{
		volume = std::stof(value);
	}
}

void FMpire::run(const float** inputs, float** outputs, uint32_t frames, const MidiEvent* midiEvents, uint32_t midiEventCount)
{
	float* const left = outputs[0];
	float* const right = outputs[1];
	
	if (sync_time)
	{
		const TimePosition& timepos = getTimePosition();
		self_frame = timepos.frame;
	}
	
	double samplerate = getSampleRate();
	
	
	float freq = 440;
	for (size_t frame = 0; frame < frames; frame++)
	{
		double time_sec = (double) self_frame / samplerate;
		left[frame] = sinf(time_sec * M_PI * 2.f * freq) * volume;
		right[frame] = left[frame];
		
		self_frame += 1;
	}
}









}
