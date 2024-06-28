#include "waveform_selector.h"

#include "draw_operations.h"
#include "wavetable.h"

#define WAVEFORM_SELECTOR_HEIGHT 10

namespace fmpire
{

WaveformSelector::WaveformSelector(Widget* parent) :
	FMpireWidget(parent),
	wavetable(nullptr),
	selected(0),
	scroll(0),
	scrolling(false),
	callback(nullptr)
{
}

WaveformSelector::~WaveformSelector() noexcept
{
}

void WaveformSelector::set_wavetable(Wavetable* const wt)
{
	wavetable = wt;
	repaint();
}

void WaveformSelector::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();
	float radius = theme->corner_radius;
	float line_width = theme->line_thin;
	float drawable_width = getWidth();
	if (wavetable)
	{
		size_t w, h;

		wavetable->get_size(w, h);
		float box_height = getHeight() / WAVEFORM_SELECTOR_HEIGHT;
		if (h > WAVEFORM_SELECTOR_HEIGHT)
		{
			drawable_width *= 0.95;
			theme->background.setFor(context);
			fill_rounded_box(context,
							 drawable_width,
							 0,
							 getWidth() * 0.05,
							 getHeight(),
							 radius,
							 line_width);
			if (scrolling)
			{
				theme->highlight.setFor(context);
			}
			else
			{
				theme->foreground.setFor(context);
			}
			float scrollbar_y = scroll / (h * box_height) * getHeight();
			draw_rounded_box(context,
							 drawable_width,
							 scrollbar_y,
							 getWidth() * 0.05,
							 (float) WAVEFORM_SELECTOR_HEIGHT / h,
							 radius,
							 line_width);
		}
		std::vector<Point<float>> line(drawable_width);

		for (size_t pos = 0; pos < h; pos++)
		{
			float rel_pos = (float) pos / (h - 1);
			for (size_t smpl = 0; smpl < drawable_width; smpl++)
			{
				float phase = (float) smpl / drawable_width;
				float val = wavetable->sample(rel_pos, phase, false, false);
				val = 1 - val * 0.5 - 0.5;
				line[smpl].setX(smpl);
				line[smpl].setY((pos + val) * box_height - scroll);
			}

			theme->highlight.setFor(context);
			draw_line_string(context, line, 1);
		}
	}
	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 drawable_width,
					 getHeight(),
					 radius,
					 line_width);
}

bool WaveformSelector::onMouse(const MouseEvent& event)
{
	if (event.button == 1 && !event.press && scrolling)
	{
		scrolling = false;
		return true;
	}
	else if (event.button == 1 && !event.press && dragging >= 0)
	{
		dragging = -1;
		return true;
	}
	if (contains(event.pos) && event.button == 1 && event.press
		&& event.pos.getX() > getWidth() * 0.95)
	{
		scrolling = true;
		return true;
	}
	else if (contains(event.pos) && event.button == 1 && event.press)
	{
		float y = event.pos.getY();
		y += scroll;
		float box_size = getHeight() / WAVEFORM_SELECTOR_HEIGHT;
		selected = y / box_size;
		if (callback)
		{
			callback->on_select(this, selected);
		}
		return true;
	}
	return false;
}

bool WaveformSelector::onMotion(const MotionEvent& event)
{
	if (dragging >= 0)
	{
		return true;
	}
	if (scrolling)
	{
		size_t w = 0, h = 0;
		if (wavetable)
		{
			wavetable->get_size(w, h);
		}
		float box_size = (float) getHeight() / WAVEFORM_SELECTOR_HEIGHT;


		scroll = std::clamp<float>(scroll, 0, box_size * h - getHeight());
		last_mouse_pos = event.absolutePos;
		return true;
	}
	return false;
}

} // namespace fmpire
