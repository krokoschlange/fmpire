#ifndef WAVEFORM_SELECTOR_H_INCLUDED
#define WAVEFORM_SELECTOR_H_INCLUDED

#include "button.h"
#include "fmpire_widget.h"
#include <vector>

namespace fmpire
{
class StateManager;
class Waveform;
class WaveformDragAndDrop;
class Wavetable;
class WavetableCreator;

class WaveformSelector : public FMpireWidget, public Button::Callback
{
public:
	WaveformSelector(Widget* parent);
	virtual ~WaveformSelector() noexcept;

	// `creator` is only used to tell which waveforms are interpolated
	void set_wavetable(Wavetable* const wt, WavetableCreator* const creator);

	struct Callback
	{
		virtual void on_select(WaveformSelector* const selector,
							   const size_t selected) = 0;
		virtual void on_waveform_moved(WaveformSelector* const selector,
									   const size_t start,
									   const size_t end) = 0;
		virtual void remove_waveform(WaveformSelector* const selector,
									 const size_t waveform) = 0;
	};

	void set_callback(Callback* const cb);

	void on_press(Button* const button) override;

	void on_release(Button* const button) override {}

	void select_waveform(size_t wf);

	size_t get_selected() const { return selected; }

protected:
	virtual void onDisplay() override;
	virtual bool onMouse(const MouseEvent& event) override;
	virtual bool onMotion(const MotionEvent& event) override;

	virtual void onPositionChanged(const PositionChangedEvent& event) override;

	virtual void onResize(const ResizeEvent& event) override;

private:
	Wavetable* wavetable;
	WavetableCreator* wavetable_creator;

	size_t selected;

	int dragging;
	size_t drop_index;
	Ref<WaveformDragAndDrop> drag_and_drop;
	Point<double> last_mouse_pos;

	Callback* callback;

	std::vector<Ref<Button>> delete_buttons;

	void update_delete_buttons();
	void update_delete_button_positions();
};

class WaveformDragAndDrop : public FMpireWidget
{
public:
	WaveformDragAndDrop(Widget* parent, Wavetable* wt, uint32_t idx);
	virtual ~WaveformDragAndDrop() noexcept;

	void update_position(Point<double> absolutePos);

protected:
	virtual void onDisplay() override;
	virtual bool onMotion(const MotionEvent& event) override;

private:
	Ref<Wavetable> wavetable;
	uint32_t waveform_idx;
};

} // namespace fmpire

#endif // SCROLL_CONTAINER_H_INCLUDED
