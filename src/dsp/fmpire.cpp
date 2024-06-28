
#include "fmpire.h"

#include "defines.h"
#include "utils.h"

#include <charconv>
#include <iostream>
#include <string>

namespace fmpire
{

FMpire::FMpire() :
	Plugin(0, 0, 1),
	sync_time(false),
	self_frame(0),
	volume(1),
	voices(init_array<Voice, FMPIRE_VOICE_COUNT>(
		Voice(oscillators, modulators, this)))
{
	std::fill(voice_map.begin(), voice_map.end(), nullptr);
	for (size_t voice_idx = 0; voice_idx < voices.size(); voice_idx++)
	{
		free_voice_queue.push(&voices[voice_idx]);
	}
	oscillators[0].enable();
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

void FMpire::initState(uint32_t index, State& state)
{
	if (index == 0)
	{
		state.hints = 0;
		state.key = KEY_EVERYTHING;
		state.defaultValue = "";
		state.label = "everything";
		state.description = "stores entire plugin state";
		std::cout << state.key << std::endl;
	}
}

String FMpire::getState(const char* key) const
{
	std::string key_str(key);
	std::string osc_prefix(KEY_OSC_PREFIX);
	std::string mod_prefix(KEY_MOD_PREFIX);

	std::cout << "getting state (dsp) " << key << std::endl;
	String data;
	if (key_str == KEY_EVERYTHING)
	{
		for (size_t osc_idx = 0; osc_idx < oscillators.size(); osc_idx++)
		{
			data += String(oscillators[osc_idx].get_state());
		}
		std::cout << "GET EVERYTHING\n";
		std::cout << data;
		std::cout << std::endl;
	}
	/*else if (key_str.compare(0, osc_prefix.size(), osc_prefix) == 0)
	{
		size_t idx_length = 0;
		size_t osc_idx =
			std::stoi(key_str.substr(osc_prefix.size()), &idx_length);
		osc_idx = std::clamp(osc_idx, 0ul, oscillators.size());
		const Oscillator& osc = oscillators[osc_idx];
		data += String(osc.get_state(key + osc_prefix.size() + idx_length + 1));
	}*/
	return data;
}

void FMpire::setState(const char* key, const char* value)
{
	d_stdout("state set (dsp) %s %s", key, value);

	std::string_view key_view(key);
	std::string_view state(value);
	if (key_view.starts_with(KEY_OSC_PREFIX))
	{
		key_view.remove_prefix(strlen(KEY_OSC_PREFIX));
		uint32_t index = 0;
		std::from_chars_result res =
			std::from_chars(key_view.data(),
							key_view.data() + key_view.size(),
							index);
		key_view.remove_prefix(res.ptr - key_view.data() + 1);
		index = std::clamp<uint32_t>(index, 0, oscillators.size());
		oscillators[index].set_state(std::string(key_view), state);
	}

	for (size_t voice_idx = 0; voice_idx < voices.size(); voice_idx++)
	{
		Voice& voice = voices[voice_idx];
		key_view = std::string_view(key);
		state = std::string_view(value);
		if (voice.is_active())
		{
			voice.set_state(key_view, state);
		}
	}
}

void FMpire::run(const float** inputs,
				 float** outputs,
				 uint32_t frames,
				 const MidiEvent* midiEvents,
				 uint32_t midiEventCount)
{
	float* const left = outputs[0];
	float* const right = outputs[1];

	std::fill(left, left + frames, 0);
	std::fill(right, right + frames, 0);

	if (sync_time)
	{
		const TimePosition& timepos = getTimePosition();
		self_frame = timepos.frame;
	}

	for (size_t event_idx = 0; event_idx < midiEventCount; event_idx++)
	{
		on_midi_event(midiEvents[event_idx]);
	}

	for (size_t voice_idx = 0; voice_idx < voices.size(); voice_idx++)
	{
		voices[voice_idx].run(outputs, frames);
	}
}

void FMpire::on_midi_event(const MidiEvent& event)
{
	uint8_t message_type = event.data[0] >> 4;
	uint8_t channel = event.data[0] & 0b1111;
	uint32_t offset = event.frame;
	switch (message_type)
	{
	case 0b1000:
	{
		int note = event.data[1] & 0b01111111;
		on_note_off(offset, note);
		break;
	}
	case 0b1001:
	{
		int note = event.data[1] & 0b01111111;
		float velocity = (event.data[2] & 0b01111111) / 127.0f;
		on_note_on(offset, note, velocity);
		break;
	}
	case 0b1010:
	{
		int note = event.data[1] & 0b01111111;
		float pressure = (event.data[2] & 0b01111111) / 127.0f;
		on_poly_aftertouch(offset, note, pressure);
	}
	case 0b1011:
	{
		on_midi_control(offset, event.data[1], event.data[2]);
		break;
	}
	case 0b1101:
	{
		float pressure = (event.data[1] & 0b01111111) / 127.0f;
		on_mono_aftertouch(offset, pressure);
	}
	case 0b1110:
	{
		uint16_t int_val = event.data[1] | (((uint16_t) event.data[2]) << 7);
		float value = (float) int_val / 16383.0f;
		on_pitch_wheel_change(offset, value);
	}
	default:
		break;
	}
}

void FMpire::on_note_off(const uint32_t offset, const int note)
{
	if (voice_map[note])
	{
		Voice* voice = voice_map[note];
		voice->stop(offset);
	}
}

void FMpire::on_note_on(const uint32_t offset,
						const int note,
						const float velocity)
{
	if (voice_map[note])
	{
		voice_map[note]->kill();
	}

	Voice* voice = nullptr;
	if (free_voice_queue.empty())
	{
		voice = &voices[0];
	}
	else
	{
		voice = free_voice_queue.front();
		free_voice_queue.pop();
	}

	if (voice->is_active())
	{
		int old_note = voice->get_note();
		voice_map[old_note] = nullptr;
		voice->kill();
	}
	voice_map[note] = voice;
	voice->start(offset, note, velocity, getSampleRate());
}

void FMpire::on_poly_aftertouch(const uint32_t offset,
								const int note,
								const float pressure)
{
}

void FMpire::on_mono_aftertouch(const uint32_t offset, const float pressure)
{
}

void FMpire::on_pitch_wheel_change(const uint32_t offset, const float value)
{
}

void FMpire::on_midi_control(const uint32_t offset,
							 const uint8_t message,
							 const uint8_t value)
{
	switch (message)
	{
	case 120:
		on_all_sound_off();
		break;
	case 123:
		on_all_notes_off();
		break;
	default:
		break;
	}
}

void FMpire::on_all_sound_off()
{
	for (Voice& voice : voices)
	{
		voice.kill();
	}
	std::fill(voice_map.begin(), voice_map.end(), nullptr);
}

void FMpire::on_all_notes_off()
{
	for (Voice& voice : voices)
	{
		voice.stop(0);
	}
}

void FMpire::on_voice_ended(Voice* const voice)
{
	voice_map[voice->get_note()] = nullptr;
	free_voice_queue.push(voice);
}

} // namespace fmpire


