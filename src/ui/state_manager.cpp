#include "state_manager.h"

#include "defines.h"
#include "fmpire_ui.h"
#include "oscillator_settings.h"
#include "wavetable_editor.h"

namespace fmpire
{

StateManager::StateManager(FMpireUI* const plugin_ui) :
	ui(plugin_ui)
{
	std::fill(oscillator_settings.begin(), oscillator_settings.end(), nullptr);
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

WavetableCreator* StateManager::get_wavetable(const size_t index) const
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


} // namespace fmpire
