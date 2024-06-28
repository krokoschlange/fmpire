#ifndef INT_EDITOR_H_INCLUDED
#define INT_EDITOR_H_INCLUDED

#include "fmpire_widget.h"

namespace fmpire
{

class IntEditor : public FMpireWidget
{
public:
	IntEditor(Widget* parent);
	virtual ~IntEditor() noexcept;

	void set_value(const int val, const bool emit_callback = false);
	void set_default_value(const int val);
	void set_limits(const int min, const int max);

	void set_tooltip(const std::string& text);
	void set_label(const std::string& text);

	struct Callback
	{
		virtual void on_value_changed(IntEditor* const editor,
									  const int value) = 0;
	};

	void set_callback(Callback* const cb);

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;

private:
	int value;
	int default_value;
	int stored_value;
	int min_value;
	int max_value;

	bool dragging;
	float scroll_value;
	float scroll_speed;
	Point<double> last_mouse_pos;

	enum class MouseState
	{
		NONE,
		REDUCE,
		CENTER,
		INCREASE,
	};
	MouseState hover_state;
	MouseState press_state;

	std::string tooltip;
	std::string label;

	Callback* callback;
};


} // namespace fmpire

#endif // INT_EDITOR_H_INCLUDED
