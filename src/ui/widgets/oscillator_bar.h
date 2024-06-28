#ifndef OSCILLATOR_BAR_H_INCLUDED
#define OSCILLATOR_BAR_H_INCLUDED

#include "button.h"
#include "grid_container.h"

#include "defines.h"

#include <array>

namespace fmpire
{
class Border;
class OscillatorSettings;
class StateManager;

class OscillatorBar : public GridContainer, public Button::Callback
{
public:
	OscillatorBar(Widget* parent, StateManager& state_mgr);
	virtual ~OscillatorBar() noexcept;

	virtual void on_press(Button* const button);
	virtual void on_release(Button* const button);

private:
	std::array<ScopedPointer<Border>, 3> borders;
	std::array<ScopedPointer<OscillatorSettings>, FMPIRE_OSC_COUNT>
		oscillator_settings;
	ScopedPointer<Button> left;
	ScopedPointer<Button> right;
	int scroll_index;

	void scroll(const int index);
};


} // namespace fmpire

#endif // OSCILLATOR_BAR_H_INCLUDED
