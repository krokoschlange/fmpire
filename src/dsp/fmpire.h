#ifndef FMPIRE_HPP_INCLUDED
#define FMPIRE_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

#include "defines.h"
#include "mod_source.h"
#include "modulator.h"
#include "oscillator.h"
#include "patch.h"
#include "triple_buffer.h"
#include "voice.h"

#include <array>
#include <atomic>
#include <mutex>
#include <optional>
#include <queue>
#include <vector>

USE_NAMESPACE_DISTRHO

namespace fmpire
{

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

	// The first FMPIRE_MACRO_COUNT parameters are the macros: host-automatable
	// modulation sources that can be routed to any target like an LFO or
	// envelope. After them come FMPIRE_METER_COUNT hidden output parameters,
	// one per route slot, that report to the UI how far the route's target is
	// currently modulated, and FMPIRE_PLAYHEAD_COUNT ones for the playhead of
	// each modulator (see publish_meters).
	void initParameter(uint32_t index, Parameter& parameter) override;
	float getParameterValue(uint32_t index) const override;
	void setParameterValue(uint32_t index, float value) override;

	void initState(uint32_t index, State& state) override;

	String getState(const char* key) const override;
	void setState(const char* key, const char* value) override;

	void run(const float** inputs,
			 float** outputs,
			 uint32_t frames,
			 const MidiEvent* midiEvents,
			 uint32_t midiEventCount) override;

private:
	// Threading: setState() is called from non-realtime threads (UI, worker,
	// host) at any time, unsynchronized with run(). The audio thread therefore
	// only ever reads a Patch, which is built completely by a state thread and
	// handed over through a triple buffer (so nothing piles up while run() isn't
	// being called). The state side keeps its own copy of the state (the members
	// below `state_mutex`), which is what patches are built from and what
	// getState() serializes.

	bool sync_time;
	uint64_t self_frame;

	float volume;

	GlobalSources global_sources;

	std::array<Voice, FMPIRE_VOICE_COUNT> voices;

	float current_bpm;

	std::array<Voice*, 128> voice_map;
	std::queue<Voice*> free_voice_queue;

	// the voice the modulation display follows
	Voice* last_started_voice;
	std::array<std::atomic<float>, FMPIRE_METER_COUNT> route_meters;
	std::array<std::atomic<float>, FMPIRE_PLAYHEAD_COUNT> playheads;

	// Audio thread, after the voices ran: reports the modulation for each route
	// slot and the playhead of each modulator in the newest voice (0 and -1
	// when there is no voice).
	void publish_meters(const Patch& patch);

	TripleBuffer<Patch> patches;

	// state side, guarded by state_mutex
	struct ModulatorShadow
	{
		bool exists = false;
		ModulatorSettings settings;
		Curve curve;
		// what the audio side gets: the baked modulator
		Modulator modulator;
	};

	mutable std::mutex state_mutex;
	std::array<OscillatorState, FMPIRE_OSC_COUNT> oscillator_states;
	std::vector<ModulatorShadow> modulator_shadows;
	std::vector<std::optional<RouteSettings>> route_shadows;
	uint32_t modulator_generation;

	// Called with state_mutex held.
	bool load_modulator(const size_t id, std::string_view& state);
	void update_modulator(const size_t id);
	void remove_modulator(const size_t id);
	bool load_route(RouteSettings settings);
	void set_modulator_state(std::string_view key, std::string_view& state);
	void set_route_state(std::string_view key, std::string_view& state);
	void restore_everything(std::string_view& state);

	// Rebuilds `patch` completely from the state side.
	void build_patch(Patch& patch) const;

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
