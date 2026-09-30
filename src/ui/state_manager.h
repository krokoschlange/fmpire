#ifndef STATE_MANAGER_H_INCLUDED
#define STATE_MANAGER_H_INCLUDED

#include "defines.h"
#include "modulation_model.h"

#include <array>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace fmpire
{
class FMpireUI;
class OscillatorSettings;
class Wavetable;
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

	WavetableCreator* get_wavetable_creator(const size_t index) const;
	Wavetable* get_wavetable(const size_t index) const;

	void edit_wavetable(const size_t index) const;

	void on_wavetable_edited(const size_t osc) const;

	ModulationModel& get_modulation() { return modulation; }

	struct MacroListener
	{
		virtual void on_macro_changed(const size_t index,
									  const float value) = 0;
	};

	void add_macro_listener(MacroListener* const listener);
	void remove_macro_listener(MacroListener* const listener);

	float get_macro(const size_t index) const;

	void on_macro_changed(const size_t index, const float value);

	void set_macro(const size_t index, const float value);
	void begin_macro_edit(const size_t index);
	void end_macro_edit(const size_t index);

	using FileBrowserCallback = std::function<void(const char* filename)>;
	void open_file_browser(FileBrowserCallback callback);

	// Opens a "save file" dialog instead of the default "open file" one.
	void open_file_browser(FileBrowserCallback callback,
						   const bool saving,
						   const char* default_name = nullptr);
	void on_file_browser_selected(const char* filename);

private:
	FMpireUI* ui;
	WavetableEditor* wavetable_editor;

	std::array<OscillatorSettings*, FMPIRE_OSC_COUNT> oscillator_settings;

	ModulationModel modulation;

	std::array<float, FMPIRE_MACRO_COUNT> macro_values;
	std::vector<MacroListener*> macro_listeners;

	FileBrowserCallback file_browser_callback;
};

} // namespace fmpire

#endif // STATE_MANAGER_H_INCLUDED
