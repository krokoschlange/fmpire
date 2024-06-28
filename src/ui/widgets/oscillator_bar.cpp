#include "oscillator_bar.h"

#include "border.h"
#include "image_button.h"
#include "oscillator_settings.h"

namespace fmpire
{

OscillatorBar::OscillatorBar(Widget* parent, StateManager& state_mgr) :
	GridContainer(parent),
	scroll_index(0)
{
	add_row(12, 0, 0, 0, 0);

	add_column(4, 0, 0, 0, 100);
	add_column(12, 0, 0, 0, 0);
	add_column(12, 0, 0, 0, 0);
	add_column(12, 0, 0, 0, 0);
	add_column(4, 0, 0, 0, 100);

	left = new ImageButton(this);
	left->set_text("left");
	left->set_callback(this);
	put(left, 0, 0);

	right = new ImageButton(this);
	right->set_text("right");
	right->set_callback(this);
	put(right, 0, 4);

	for (size_t border_idx = 0; border_idx < borders.size(); border_idx++)
	{
		borders[border_idx] = new Border(this);
		put(borders[border_idx], 0, border_idx + 1);
	}

	for (size_t osc_idx = 0; osc_idx < oscillator_settings.size(); osc_idx++)
	{
		oscillator_settings[osc_idx] =
			new OscillatorSettings(this, osc_idx, state_mgr);
	}
	scroll(0);
}

OscillatorBar::~OscillatorBar() noexcept
{
}

void OscillatorBar::on_press(Button* const button)
{
	if (button == left)
	{
		scroll(scroll_index - 1);
	}
	else if (button == right)
	{
		scroll(scroll_index + 1);
	}
}

void OscillatorBar::on_release(Button* const button)
{
}

void OscillatorBar::scroll(const int index)
{
	scroll_index = std::clamp(index, 0, (int) oscillator_settings.size() - 3);
	for (size_t osc_idx = 0; osc_idx < oscillator_settings.size(); osc_idx++)
	{
		if (osc_idx < scroll_index || osc_idx > scroll_index + 2)
		{
			oscillator_settings[osc_idx]->hide();
			continue;
		}
		oscillator_settings[osc_idx]->show();
		put(oscillator_settings[osc_idx], 0, osc_idx - scroll_index + 1);
	}
	PositionChangedEvent pos_event;
	pos_event.pos = getAbsolutePos();
	pos_event.oldPos = getAbsolutePos();
	onPositionChanged(pos_event);
	ResizeEvent res_event;
	res_event.size = getSize();
	res_event.oldSize = getSize();
	onResize(res_event);
	repaint();
}


} // namespace fmpire
