#ifndef INT_EDITOR_H_INCLUDED
#define INT_EDITOR_H_INCLUDED

#include "fmpire_widget.h"

namespace fmpire
{

class IntEditor : public FMpireWidget
{
public:
	enum class LabelPosition
	{
		BELOW,
		LEFT,
	};

	IntEditor(Widget* parent);
	virtual ~IntEditor() noexcept;

	void set_value(const int val, const bool emit_callback = false);
	void set_default_value(const int val);
	void set_limits(const int min, const int max);

	void set_tooltip(const std::string& text);
	void set_label(const std::string& text);
	void set_label_position(const LabelPosition position,
							const float proportion = 0.0f);

	int get_value() const { return value; }

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
	bool onCharacterInput(const CharacterInputEvent& event) override;
	bool onKeyboard(const KeyboardEvent& event) override;

	void on_focus_lost() override;

private:
	int value;
	int default_value;
	int stored_value;
	int min_value;
	int max_value;

	bool dragging;
	bool drag_moved;
	float scroll_value;
	float scroll_speed;
	Point<double> press_mouse_pos;
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

	MouseState get_zone(const double x) const;
	float get_box_left() const;
	void update_left_label_metrics(const GraphicsContext& context);

	void begin_edit();
	void commit_edit();
	void cancel_edit();

	bool editing;
	bool edit_replaces_text;
	std::string edit_text;

	std::string tooltip;
	std::string label;
	LabelPosition label_position;
	float label_proportion;
	float left_label_width;
	float left_label_size;

	Callback* callback;
};


} // namespace fmpire

#endif // INT_EDITOR_H_INCLUDED
