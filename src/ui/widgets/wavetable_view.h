#ifndef WAVETABLE_VIEW_H_INCLUDED
#define WAVETABLE_VIEW_H_INCLUDED

#include "button.h"
#include "relative_container.h"

namespace fmpire
{
class StateManager;
class Wavetable;

class WavetableView : public RelativeContainer, public Button::Callback
{
public:
	WavetableView(Widget* parent,
				  const size_t idx,
				  const Wavetable& wt,
				  StateManager& state_mgr);
	virtual ~WavetableView() noexcept;

	void on_press(Button* const button) override;
	void on_release(Button* const button) override;

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;

private:
	size_t index;
	bool single;

	ScopedPointer<Button> edit_button;
	const Wavetable& wavetable;
	StateManager& state_manager;
};

} // namespace fmpire

#endif // WAVETABLE_VIEW_H_INCLUDED
