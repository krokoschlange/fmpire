#include "scroll_container.h"
#include "draw_operations.h"
#include <cstdlib>

namespace fmpire
{

ScrollContainer::ScrollContainer(Widget* parent) :
	FMpireWidget(parent),
	scroll_mode((ScrollMode) (VERTICAL | HORIZONTAL)),
	vertical_scroll(0),
	horizontal_scroll(0),
	scroll_width(100),
	scroll_height(100),
	is_dragging_horizontal(false),
	is_dragging_vertical(false)
{
}

ScrollContainer::~ScrollContainer() noexcept
{
}

void ScrollContainer::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	theme->background.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 theme->line_very_thin);

	if (scroll_mode & VERTICAL)
	{
		float bar_width = std::min(getWidth() * 0.05f, 10.0f);

		float bar_height = getHeight();
		if (scroll_mode & HORIZONTAL)
		{
			bar_height =
				std::max(0.0f,
						 bar_height - std::min(getHeight() * 0.05f, 10.0f));
		}

		theme->background.setFor(context);
		draw_rounded_box(context,
						 getWidth() - bar_width,
						 0,
						 bar_width,
						 bar_height,
						 theme->corner_radius,
						 theme->line_very_thin);

		if (scroll_height > bar_height)
		{
			float handle_height =
				std::max(bar_height * (bar_height / scroll_height), 10.0f);
			float handle_pos = (bar_height - handle_height) * vertical_scroll
							 / (scroll_height - bar_height);

			theme->foreground.setFor(context);
			fill_rounded_box(context,
							 getWidth() - bar_width,
							 handle_pos,
							 bar_width,
							 handle_height,
							 theme->corner_radius,
							 theme->line_thin);
		}
	}

	if (scroll_mode & HORIZONTAL)
	{
		float bar_height = std::min(getWidth() * 0.05f, 10.0f);

		float bar_width = getWidth();
		if (scroll_mode & VERTICAL)
		{
			bar_width =
				std::max(0.0f, bar_width - std::min(getWidth() * 0.05f, 10.0f));
		}

		theme->background.setFor(context);
		draw_rounded_box(context,
						 0,
						 getHeight() - bar_height,
						 bar_width,
						 bar_height,
						 theme->corner_radius,
						 theme->line_very_thin);

		if (scroll_width > bar_width)
		{
			float handle_width =
				std::max(bar_width * (bar_width / scroll_width), 10.0f);
			float handle_pos = (bar_width - handle_width) * horizontal_scroll
							 / (scroll_width - bar_width);

			theme->foreground.setFor(context);
			fill_rounded_box(context,
							 handle_pos,
							 getHeight() - bar_height,
							 handle_width,
							 bar_height,
							 theme->corner_radius,
							 theme->line_thin);
		}
	}
}

void ScrollContainer::onPositionChanged(const PositionChangedEvent& event)
{
	update_child_position();
}

void ScrollContainer::onResize(const ResizeEvent& event)
{
	update_child_position();
}

bool ScrollContainer::onMouse(const MouseEvent& event)
{
	if (event.press && contains_clipped(event.pos))
	{
		if (scroll_mode & VERTICAL)
		{
			float bar_width = std::min(getWidth() * 0.05f, 10.0f);
			float bar_height = getHeight();
			if (scroll_mode & HORIZONTAL)
			{
				bar_height =
					std::max(0.0f,
							 bar_height - std::min(getHeight() * 0.05f, 10.0f));
			}

			if (scroll_height > bar_height)
			{
				float handle_height =
					std::max(bar_height * (bar_height / scroll_height), 10.0f);
				float handle_pos = (bar_height - handle_height)
								 * vertical_scroll
								 / (scroll_height - bar_height);

				if (Rectangle<double>(getWidth() - bar_width,
									  handle_pos,
									  bar_width,
									  handle_height)
						.contains(event.pos))
				{
					is_dragging_vertical = true;
					drag_mouse_start = event.pos.getY();
					drag_scroll_start = vertical_scroll;
					return true;
				}
			}
		}
		if (scroll_mode & HORIZONTAL)
		{
			float bar_height = std::min(getWidth() * 0.05f, 10.0f);

			float bar_width = getWidth();
			if (scroll_mode & VERTICAL)
			{
				bar_width =
					std::max(0.0f,
							 bar_width - std::min(getWidth() * 0.05f, 10.0f));
			}

			if (scroll_width > bar_width)
			{
				float handle_width =
					std::max(bar_width * (bar_width / scroll_width), 10.0f);
				float handle_pos = (bar_width - handle_width)
								 * horizontal_scroll
								 / (scroll_width - bar_width);

				if (Rectangle<double>(handle_pos,
									  getHeight() - bar_height,
									  handle_width,
									  bar_height)
						.contains(event.pos))
				{
					is_dragging_horizontal = true;
					drag_mouse_start = event.pos.getX();
					drag_scroll_start = horizontal_scroll;
					return true;
				}
			}
		}
	}
	else if (!event.press && (is_dragging_horizontal || is_dragging_vertical))
	{
		is_dragging_horizontal = false;
		is_dragging_vertical = false;
		return true;
	}
	return FMpireWidget::onMouse(event);
}

bool ScrollContainer::onMotion(const MotionEvent& event)
{
	if (is_dragging_vertical)
	{
		float mouse_delta = event.pos.getY() - drag_mouse_start;

		float bar_height = getHeight();
		if (scroll_mode & HORIZONTAL)
		{
			bar_height =
				std::max(0.0f,
						 bar_height - std::min(getHeight() * 0.05f, 10.0f));
		}

		if (bar_height < scroll_height)
		{
			float handle_height =
				std::max(bar_height * (bar_height / scroll_height), 10.0f);

			float scroll_delta = mouse_delta / (bar_height - handle_height)
							   * (scroll_height - bar_height);

			vertical_scroll = drag_scroll_start + scroll_delta;
			vertical_scroll = std::min(std::max(0.0f, vertical_scroll),
									   scroll_height - bar_height);
			repaint();
			return true;
		}
	}
	else if (is_dragging_horizontal)
	{
		float mouse_delta = event.pos.getX() - drag_mouse_start;

		float bar_height = std::min(getWidth() * 0.05f, 10.0f);

		float bar_width = getWidth();
		if (scroll_mode & VERTICAL)
		{
			bar_width =
				std::max(0.0f, bar_width - std::min(getWidth() * 0.05f, 10.0f));
		}

		if (bar_width < scroll_width)
		{
			float handle_width =
				std::max(bar_width * (bar_width / scroll_width), 10.0f);

			float scroll_delta = mouse_delta / (bar_width - handle_width)
							   * (scroll_width - bar_width);

			horizontal_scroll = drag_scroll_start + scroll_delta;
			horizontal_scroll = std::min(std::max(0.0f, horizontal_scroll),
										 scroll_width - bar_width);
			repaint();
			return true;
		}
	}
	return FMpireWidget::onMotion(event);
}

bool ScrollContainer::onScroll(const ScrollEvent& event)
{
	if (!contains_clipped(event.pos))
	{
		return FMpireWidget::onScroll(event);
	}

	if (scroll_mode & VERTICAL)
	{
		float bar_height = getHeight();
		if (scroll_mode & HORIZONTAL)
		{
			bar_height =
				std::max(0.0f,
						 bar_height - std::min(getHeight() * 0.05f, 10.0f));
		}

		if (bar_height < scroll_height)
		{
			vertical_scroll -= event.delta.getY() * 30;
			vertical_scroll = std::min(std::max(vertical_scroll, 0.0f),
									   scroll_height - bar_height);
			repaint();
			return true;
		}
	}
	if (scroll_mode & HORIZONTAL)
	{
		float bar_width = getWidth();
		if (scroll_mode & VERTICAL)
		{
			bar_width =
				std::max(0.0f, bar_width - std::min(getWidth() * 0.05f, 10.0f));
		}

		if (bar_width < scroll_width)
		{
			horizontal_scroll -= event.delta.getX() * 30;
			if ((scroll_mode & VERTICAL) == 0)
			{
				horizontal_scroll -= event.delta.getY() * 30;
			}
			horizontal_scroll = std::min(std::max(0.0f, horizontal_scroll),
										 scroll_width - bar_width);
			repaint();
			return true;
		}
	}

	return FMpireWidget::onScroll(event);
}

void ScrollContainer::update_child_position()
{
	if (getChildren().empty())
	{
		return;
	}

	FMpireWidget* const child = (FMpireWidget*) getChildren().front();

	Point<int> child_pos = getAbsolutePos();
	unsigned int child_width = getWidth();
	unsigned int child_height = getHeight();

	if (scroll_mode & VERTICAL)
	{
		child_pos.setY(child_pos.getY() - vertical_scroll);
		child_height = scroll_height;

		if ((scroll_mode & HORIZONTAL) == 0)
		{
			child_width = getWidth() - std::min(getWidth() * 0.05f, 10.0f);
		}
	}

	if (scroll_mode & HORIZONTAL)
	{
		child_pos.setX(child_pos.getX() - horizontal_scroll);
		child_width = scroll_width;

		if ((scroll_mode & VERTICAL) == 0)
		{
			child_height = getHeight() - std::min(getHeight() * 0.05f, 10.0f);
		}
	}

	child->setAbsolutePos(child_pos);
	child->setSize(child_width, child_height);

	float clip_width = getWidth();
	float clip_height = getHeight();
	Corner clip_corners = Corner::TOP_LEFT;

	if (scroll_mode & VERTICAL)
	{
		clip_width -= std::min(getWidth() * 0.05f, 10.0f);
	}
	else
	{
		clip_corners |= Corner::TOP_RIGHT;
	}

	if (scroll_mode & HORIZONTAL)
	{
		clip_height -= std::min(getHeight() * 0.05f, 10.0f);
	}
	else
	{
		clip_corners |= Corner::BOTTOM_LEFT;
	}

	if (scroll_mode == 0)
	{
		clip_corners |= Corner::BOTTOM_RIGHT;
	}

	child->set_clip_area(Rectangle<float>(getAbsoluteX(),
										  getAbsoluteY(),
										  clip_width,
										  clip_height),
						 theme->corner_radius,
						 clip_corners);
}

} // namespace fmpire
