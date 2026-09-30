#ifndef WAVETABLE_VIEW_H_INCLUDED
#define WAVETABLE_VIEW_H_INCLUDED

#include "button.h"
#include "modulation_model.h"
#include "relative_container.h"

namespace fmpire
{
class StateManager;
class Wavetable;

class WavetableView :
	public RelativeContainer,
	public Button::Callback,
	public ModulationModel::Listener
{
public:
	WavetableView(Widget* parent,
				  const size_t idx,
				  const Wavetable& wt,
				  StateManager& state_mgr);
	virtual ~WavetableView() noexcept;

	void on_press(Button* const button) override;
	void on_release(Button* const button) override;

	// The position of the knob; the view highlights the wavetable position
	// that is actually playing, i.e. the knob modulated by its routes.
	void set_wavetable_pos(float wt_pos);

	void on_modulation_changed() override;
	void on_route_meter_changed(const size_t slot) override;

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;

private:
	size_t index;
	bool single;

	float wavetable_pos;

	// the position that is playing right now
	float current_pos() const;

	Ref<Button> edit_button;
	const Wavetable& wavetable;
	StateManager& state_manager;
};

} // namespace fmpire

#endif // WAVETABLE_VIEW_H_INCLUDED
