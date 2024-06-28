#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

#include "fmpire_widget.h"

#include <string>

namespace fmpire
{

class Button : public FMpireWidget
{
public:
	Button(Widget* parent);
	virtual ~Button() noexcept;

	void set_toggle(const bool toggle);
	void set_text(const std::string& txt);
	void set_text_y_scale(const float scale);
	void set_enabled(const bool enabled);
	void set_pressed(const bool pressed);

	struct Callback
	{
		virtual void on_press(Button* const button) = 0;
		virtual void on_release(Button* const button) = 0;
	};

	void set_callback(Callback* cb);

protected:
	virtual void onDisplay() override;
	bool onMouse(const MouseEvent&) override;
	bool onMotion(const MotionEvent&) override;

	enum class State
	{
		DISABLED,
		NORMAL,
		HOVER,
		PRESS
	};

	State state;
	bool is_toggle;
	std::string text;
	float text_y_scale;
	Callback* callback;
};

} // namespace fmpire

#endif // BUTTON_H_INCLUDED
