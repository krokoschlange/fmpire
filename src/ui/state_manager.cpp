#include "state_manager.h"

#include <algorithm>

#include "defines.h"
#include "fmpire_ui.h"
#include "oscillator_settings.h"
#include "wavetable_editor.h"

namespace fmpire
{

StateManager::StateManager(FMpireUI* const plugin_ui) :
	ui(plugin_ui),
	modulation(*this)
{
	std::fill(oscillator_settings.begin(), oscillator_settings.end(), nullptr);
	macro_values.fill(0.0f);
}

StateManager::~StateManager() noexcept
{
}

void StateManager::set_state(const std::string& key, const std::string& value)
{
	ui->setState(key.c_str(), value.c_str());
}

void StateManager::state_changed(const std::string& key,
								 std::string_view& state)
{
	if (key == KEY_EVERYTHING)
	{
		for (size_t osc_idx = 0; osc_idx < oscillator_settings.size();
			 osc_idx++)
		{
			if (oscillator_settings[osc_idx])
			{
				oscillator_settings[osc_idx]->set_state(state);
			}
		}

		modulation.parse_state(state);
	}
}

void StateManager::add(OscillatorSettings* const osc_settings, size_t index)
{
	if (index < oscillator_settings.size())
	{
		oscillator_settings[index] = osc_settings;
	}
}

void StateManager::set_wavetable_editor(WavetableEditor* const editor)
{
	wavetable_editor = editor;
}

WavetableCreator* StateManager::get_wavetable_creator(const size_t index) const
{
	OscillatorSettings* osc_settings = oscillator_settings[index];
	if (osc_settings)
	{
		return osc_settings->get_wavetable_creator();
	}
	return nullptr;
}

Wavetable* StateManager::get_wavetable(const size_t index) const
{
	OscillatorSettings* osc_settings = oscillator_settings[index];
	if (osc_settings)
	{
		return osc_settings->get_wavetable();
	}
	return nullptr;
}

void StateManager::edit_wavetable(const size_t index) const
{
	wavetable_editor->select_oscillator(index);
	ui->switch_to_tab(3);
}

void StateManager::on_wavetable_edited(const size_t osc) const
{
	if (osc >= oscillator_settings.size())
	{
		return;
	}

	oscillator_settings[osc]->on_wavetable_changed();
}

void StateManager::add_macro_listener(MacroListener* const listener)
{
	macro_listeners.push_back(listener);
}

void StateManager::remove_macro_listener(MacroListener* const listener)
{
	macro_listeners.erase(
		std::remove(macro_listeners.begin(), macro_listeners.end(), listener),
		macro_listeners.end());
}

float StateManager::get_macro(const size_t index) const
{
	return index < macro_values.size() ? macro_values[index] : 0.0f;
}

void StateManager::on_macro_changed(const size_t index, const float value)
{
	if (index >= macro_values.size())
	{
		return;
	}
	macro_values[index] = value;

	const std::vector<MacroListener*> listeners = macro_listeners;
	for (MacroListener* const listener : listeners)
	{
		listener->on_macro_changed(index, value);
	}
}

void StateManager::set_macro(const size_t index, const float value)
{
	if (index >= macro_values.size())
	{
		return;
	}
	ui->setParameterValue(index, value);
	on_macro_changed(index, value);
}

void StateManager::begin_macro_edit(const size_t index)
{
	if (index < macro_values.size())
	{
		ui->editParameter(index, true);
	}
}

void StateManager::end_macro_edit(const size_t index)
{
	if (index < macro_values.size())
	{
		ui->editParameter(index, false);
	}
}

void StateManager::open_file_browser(FileBrowserCallback callback)
{
	file_browser_callback = callback;
	ui->openFileBrowser();
}

void StateManager::open_file_browser(FileBrowserCallback callback,
									 const bool saving,
									 const char* default_name)
{
	file_browser_callback = callback;

	DISTRHO_NAMESPACE::FileBrowserOptions options;
	options.saving = saving;
	options.defaultName = default_name;
	ui->openFileBrowser(options);
}

void StateManager::on_file_browser_selected(const char* filename)
{
	if (file_browser_callback)
	{
		FileBrowserCallback callback = file_browser_callback;
		file_browser_callback = nullptr;
		callback(filename);
	}
}

} // namespace fmpire
