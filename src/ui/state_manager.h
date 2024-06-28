#ifndef STATE_MANAGER_H_INCLUDED
#define STATE_MANAGER_H_INCLUDED

#include "defines.h"

#include <array>
#include <string>
#include <string_view>

namespace fmpire
{
class FMpireUI;
class OscillatorSettings;
class WavetableCreator;
class WavetableEditor;

class StateManager
{
public:
	StateManager(FMpireUI* const plugin_ui);
	virtual ~StateManager() noexcept;

	void state_changed(const std::string& key, std::string_view& state);
	void set_state(const std::string& key, const std::string& value);

	void add(OscillatorSettings* const osc_settings, size_t index);
	void set_wavetable_editor(WavetableEditor* const editor);

	WavetableCreator* get_wavetable(const size_t index) const;

	void edit_wavetable(const size_t index) const;

private:
	FMpireUI* ui;
	WavetableEditor* wavetable_editor;

	std::array<OscillatorSettings*, FMPIRE_OSC_COUNT> oscillator_settings;
};

} // namespace fmpire

#endif // STATE_MANAGER_H_INCLUDED
