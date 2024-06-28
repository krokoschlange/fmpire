#include "tooltip.h"

#include "draw_operations.h"

#include <chrono>
#include <iostream>

namespace fmpire
{

Tooltip::Tooltip(Widget* parent) :
	FMpireWidget(parent),
	text_size(0.03),
	requested(false),
	pinned(false),
	last_mouse_pos(0, 0),
	last_time(0),
	delay(500)
{
	setNeedsFullViewportDrawing(true);
}

Tooltip::~Tooltip() noexcept
{
}

void Tooltip::request(const std::string& txt,
					  const float x,
					  const float y,
					  const bool pin)
{
	if (pinned)
	{
		return;
	}
	text = txt;
	setAbsolutePos(x, y);
	requested = true;
	pinned = pin;
}

void Tooltip::set_text(const std::string& txt)
{
	text = txt;
	repaint();
}

void Tooltip::set_text_size(const float size)
{
	text_size = size;
	repaint();
}

void Tooltip::unpin()
{
	pinned = false;
}

void Tooltip::idle()
{
	std::chrono::steady_clock::time_point now =
		std::chrono::steady_clock::now();
	uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>(
						now.time_since_epoch())
						.count();
	if ((pinned || time - last_time > delay) && requested)
	{
		requested = false;
		show();
		repaint();
	}
}

void Tooltip::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();

	Widget* parent = getParentWidget();

	float font_size = parent->getHeight() * text_size;
	float width, height;
	get_text_rect(context,
				  text.c_str(),
				  theme->font.c_str(),
				  font_size,
				  width,
				  height);
	width += font_size;
	height += font_size;

	float x = getAbsoluteX() + 20;
	float y = getAbsoluteY();

	y = std::min(y, parent->getHeight() - height);
	if (x + width > parent->getWidth())
	{
		x = getAbsoluteX() - 20 - width;
	}

	theme->background.setFor(context);
	fill_rounded_box(context,
					 x,
					 y,
					 width,
					 height,
					 theme->corner_radius,
					 theme->line_thin);

	Color(255, 255, 255).setFor(context);
	draw_text(context,
			  text.c_str(),
			  theme->font.c_str(),
			  font_size,
			  Anchor::CENTER,
			  x + width / 2,
			  y + height / 2);

	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 x,
					 y,
					 width,
					 height,
					 theme->corner_radius,
					 theme->line_thin);
}

bool Tooltip::handle_motion(const MotionEvent& event)
{
	last_mouse_pos = event.absolutePos;
	std::chrono::steady_clock::time_point now =
		std::chrono::steady_clock::now();
	last_time = std::chrono::duration_cast<std::chrono::milliseconds>(
					now.time_since_epoch())
					.count();
	if (!pinned)
	{
		requested = false;
		hide();
		getWindow().repaint();
	}
	return false;
}

} // namespace fmpire
