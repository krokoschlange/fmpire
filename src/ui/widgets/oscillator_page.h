#ifndef OSCILLATOR_PAGE_H_INCLUDED
#define OSCILLATOR_PAGE_H_INCLUDED

#include "grid_container.h"
#include "modulator_editor.h"
#include "oscillator_bar.h"

namespace fmpire
{
class StateManager;

class OscillatorPage : public GridContainer
{
public:
	OscillatorPage(Widget* parent, StateManager& state_mgr);
	virtual ~OscillatorPage() noexcept;

private:
	Ref<OscillatorBar> oscillator_bar;
	Ref<ModulatorEditor> modulator_editor;
};

} // namespace fmpire

#endif // OSCILLATOR_PAGE_H_INCLUDED
