#include "wavetable_view.h"

#include "draw_operations.h"
#include "state_manager.h"
#include "wavetable.h"

#include <iostream>

namespace fmpire
{

WavetableView::WavetableView(Widget* parent,
							 const size_t idx,
							 const Wavetable& wt,
							 StateManager& state_mgr) :
	RelativeContainer(parent),
	index(idx),
	single(false),
	wavetable(wt),
	state_manager(state_mgr)
{
	edit_button = new Button(this);
	edit_button->set_text("Edit");
	edit_button->set_callback(this);
	put(edit_button, 0.80, 0.05, 0.15, 0.15);
}

WavetableView::~WavetableView() noexcept
{
}

void WavetableView::on_press(Button* const button)
{
	if (button == edit_button)
	{
		state_manager.edit_wavetable(index);
	}
}

void WavetableView::on_release(Button* const button)
{
}

void draw_waveform(const GraphicsContext& context,
				   const Wavetable& wavetable,
				   const float position,
				   const float x,
				   const float y,
				   const float width,
				   const float height)
{
	size_t w, h;
	wavetable.get_size(w, h);
	w = std::max<size_t>(2, w);
	size_t smpls = w;
	std::vector<Point<float>> line(smpls);
	for (size_t smpl = 0; smpl < smpls; smpl++)
	{
		float phase = (float) smpl / (float) (smpls - 1);
		float value = 1 - wavetable.sample(position, phase) * 0.5 - 0.5;
		line[smpl].setX(x + phase * width);
		line[smpl].setY(y + value * height);
	}
	draw_line_string(context, line, 1);
}

void WavetableView::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();
	float radius = theme->corner_radius;
	float line_width = theme->line_thin;


	if (single)
	{
		float pos = 0;

		theme->highlight.setFor(context);
		draw_waveform(context, wavetable, pos, 0, 0, getWidth(), getHeight());
	}
	else
	{
		size_t w, h;
		wavetable.get_size(w, h);

		Color alpha_bg = theme->background;
		alpha_bg.alpha = 0.1;
		alpha_bg.setFor(context, true);
		for (size_t pos = 0; pos < h; pos++)
		{
			float rel_pos = (float) pos / (h - 1);
			float x = rel_pos * getWidth() * 0.3;
			float y = (1 - rel_pos) * getHeight() * 0.5;
			draw_waveform(context,
						  wavetable,
						  rel_pos,
						  x,
						  y,
						  getWidth() * 0.7,
						  getHeight() * 0.5);
		}
	}

	theme->background.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 radius,
					 line_width);
}

bool WavetableView::onMouse(const MouseEvent& event)
{
	if (RelativeContainer::onMouse(event))
	{
		return true;
	}
	if (contains(event.pos) && event.button == 1 && event.press)
	{
		single = !single;
		repaint();
		return true;
	}
	return false;
}


} // namespace fmpire
