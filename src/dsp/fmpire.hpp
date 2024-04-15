#ifndef FMPIRE_HPP_INCLUDED
#define FMPIRE_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

USE_NAMESPACE_DISTRHO

namespace fmpire
{

class FMpire : public Plugin
{
public:
	FMpire();
	virtual ~FMpire() noexcept;

protected:
	const char* getLabel() const override;
	const char* getDescription() const override;
	const char* getMaker() const override;
	const char* getHomePage() const override;
	const char* getLicense() const override;
	uint32_t getVersion() const override;
	int64_t getUniqueId() const override;

	String getState(const char* key) const override;
	void setState(const char* key, const char* value) override;

	void run(const float** inputs,
			 float** outputs,
			 uint32_t frames,
			 const MidiEvent* midiEvents,
			 uint32_t midiEventCount) override;

private:
	bool sync_time;
	uint64_t self_frame;

	float volume;
};

} // namespace fmpire

START_NAMESPACE_DISTRHO

Plugin* createPlugin()
{
	return new fmpire::FMpire();
}

END_NAMESPACE_DISTRHO

#endif // FMPIRE_HPP_INCLUDED
