#ifndef SCROLL_CONTAINER_H_INCLUDED
#define SCROLL_CONTAINER_H_INCLUDED

#include "fmpire_widget.h"

namespace fmpire
{
class Wavetable;

class WaveformSelector : public FMpireWidget
{
public:
	WaveformSelector(Widget* parent);
	virtual ~WaveformSelector() noexcept;

	void set_wavetable(Wavetable* const wt);

	struct Callback
	{
		virtual void on_select(WaveformSelector* const selector, const size_t selected);
		virtual void on_waveform_moved(WaveformSelector* const selector,
							   const size_t start,
							   const size_t end);
	};

	void set_callback(Callback* const cb);

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;

private:
	Wavetable* wavetable;

	size_t selected;
	float scroll;

	bool scrolling;

	int dragging;
	Point<double> last_mouse_pos;

	Callback* callback;
};

} // namespace fmpire

#endif // SCROLL_CONTAINER_H_INCLUDED
