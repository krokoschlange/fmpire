#ifndef FMPIRE_KNOB_H_INCLUDED
#define FMPIRE_KNOB_H_INCLUDED

#include "DistrhoUI.hpp"
#include "fmpire_widget.h"

#include "theme.h"

namespace fmpire
{

class Knob : public SubWidget, public FMpireWidget
{
public:
	explicit Knob(Widget* parentWidget);
	virtual ~Knob() noexcept;

	void set_value(float new_value, bool emit_callback = false);

	struct Callback
	{
	public:
		virtual void drag_started(Knob* const knob) = 0;
		virtual void drag_ended(Knob* const knob) = 0;
		virtual void value_changed(Knob* const knob, float value) = 0;
	};

	void set_callback(Callback* cb);

protected:
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;

	void onDisplay() override;

private:
	float value;
	float default_value;
	float stored_value;

	float drag_speed;
	bool dragging;
	Point<double> last_mouse_pos;

	Callback* callback;
};


} // namespace fmpire

#endif // FMPIRE_KNOB_H_INCLUDED
