#ifndef FMPIRE_KNOB_H_INCLUDED
#define FMPIRE_KNOB_H_INCLUDED

#include "DistrhoUI.hpp"
#include "fmpire_widget.h"

#include "fmpire_theme.h"

namespace fmpire
{

class FMpireKnob : public SubWidget, public FMpireWidget
{
public:
	explicit FMpireKnob(Widget* parentWidget);
	virtual ~FMpireKnob() noexcept;
	
	void set_value(float new_value, bool emit_callback = false);
	
	struct Callback
	{
	public:
		virtual void drag_started(FMpireKnob* const knob) = 0;
		virtual void drag_ended(FMpireKnob* const knob) = 0;
		virtual void value_changed(FMpireKnob* const knob, float value) = 0;
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


}

#endif // FMPIRE_KNOB_H_INCLUDED
