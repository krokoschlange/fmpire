#ifndef SCROLL_CONTAINER_H_INCLUDED
#define SCROLL_CONTAINER_H_INCLUDED

#include "fmpire_widget.h"

namespace fmpire
{

class ScrollContainer : public FMpireWidget
{
public:
	ScrollContainer(Widget* parent);
	virtual ~ScrollContainer() noexcept;

	enum ScrollMode
	{
		VERTICAL = 1,
		HORIZONTAL = 2,
	};

	inline void set_scroll_mode(ScrollMode mode) { scroll_mode = mode; }

	inline ScrollMode get_scroll_mode() const { return scroll_mode; }

	virtual void set_clip_area(Rectangle<float> area,
							   const float cr,
							   const Corner cc) override
	{
		clip_area = area;
		clip_radius = cr;
		clip_corners = cc;
	}

	inline void set_scroll_area(float width, float height)
	{
		scroll_width = width;
		scroll_height = height;
		update_child_position();
		repaint();
	}

	inline float get_scroll_width() const { return scroll_width; }

	inline float get_scroll_height() const { return scroll_height; }

protected:
	virtual void onDisplay() override;
	virtual void onPositionChanged(const PositionChangedEvent& event) override;
	virtual void onResize(const ResizeEvent& event) override;

	virtual bool onMouse(const MouseEvent& event) override;
	virtual bool onMotion(const MotionEvent& event) override;
	virtual bool onScroll(const ScrollEvent& event) override;

private:
	ScrollMode scroll_mode;
	float horizontal_scroll;
	float vertical_scroll;

	float scroll_width;
	float scroll_height;

	bool is_dragging_horizontal;
	bool is_dragging_vertical;
	float drag_mouse_start;
	float drag_scroll_start;

	void update_child_position();
};

} // namespace fmpire

#endif // SCROLL_CONTAINER_H_INCLUDED
