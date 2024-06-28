#ifndef FMPIRE_HPP_INCLUDED
#define FMPIRE_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

#include "defines.h"
#include "oscillator.h"
#include "voice.h"

#include <array>
#include <queue>
#include <vector>

USE_NAMESPACE_DISTRHO

namespace fmpire
{
class Modulator;

class FMpire : public Plugin, public Voice::VoiceEndedCallback
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

	void initState(uint32_t index, State& state) override;

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

	std::array<Oscillator, FMPIRE_OSC_COUNT> oscillators;
	std::array<Voice, FMPIRE_VOICE_COUNT> voices;
	std::vector<std::shared_ptr<Modulator>> modulators;

	std::array<Voice*, 128> voice_map;
	std::queue<Voice*> free_voice_queue;

	void on_midi_event(const MidiEvent& event);
	void on_note_off(const uint32_t offset, const int note);
	void on_note_on(const uint32_t offset,
					const int note,
					const float velocity);
	void on_poly_aftertouch(const uint32_t offset,
							const int note,
							const float pressure);
	void on_mono_aftertouch(const uint32_t offset, const float pressure);
	void on_pitch_wheel_change(const uint32_t offset, const float value);

	void on_midi_control(const uint32_t offset,
						 const uint8_t message,
						 const uint8_t value);
	void on_all_sound_off();
	void on_all_notes_off();

	void on_voice_ended(Voice* const voice) override;
};

} // namespace fmpire

START_NAMESPACE_DISTRHO

Plugin* createPlugin()
{
	return new fmpire::FMpire();
}

END_NAMESPACE_DISTRHO

#endif // FMPIRE_HPP_INCLUDED
