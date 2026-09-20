#include "oscillator_page.h"

namespace fmpire
{

OscillatorPage::OscillatorPage(Widget* parent, StateManager& state_mgr) :
	GridContainer(parent)
{
	add_row(3, 0, 0, 200, 0);
	add_row(2, 0, 0, 200, 0);
	add_column(1, 0, 0, 0, 0);

	oscillator_bar = new OscillatorBar(this, state_mgr);
	put(oscillator_bar, 0, 0);

	modulator_editor = new ModulatorEditor(this, state_mgr);
	put(modulator_editor, 1, 0);
}

OscillatorPage::~OscillatorPage() noexcept
{
}

} // namespace fmpire
