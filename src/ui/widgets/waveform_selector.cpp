#include "waveform_selector.h"

#include "Base.hpp"
#include "draw_operations.h"
#include "fmpire_window.h"
#include "wavetable.h"
#include "wavetable_creator.h"

#define WAVEFORM_BOX_HEIGHT 40
#define WAVEFORM_DRAG_THRESHOLD 5

namespace fmpire
{

WaveformSelector::WaveformSelector(Widget* parent) :
	FMpireWidget(parent),
	wavetable(nullptr),
	wavetable_creator(nullptr),
	selected(0),
	dragging(-2),
	callback(nullptr)
{
}

WaveformSelector::~WaveformSelector() noexcept
{
}

void WaveformSelector::set_wavetable(Wavetable* const wt,
									 WavetableCreator* const creator)
{
	wavetable = wt;
	wavetable_creator = creator;
	selected = 0;
	repaint();
}

void WaveformSelector::set_callback(Callback* const cb)

{
	callback = cb;
}

void WaveformSelector::on_press(Button* const button)
{
	for (size_t i = 0; i < delete_buttons.size(); i++)
	{
		if ((Button*) delete_buttons[i] == button)
		{
			if (callback)
			{
				callback->remove_waveform(this, i);
				select_waveform(i);
				repaint();
			}
			return;
		}
	}
}

void WaveformSelector::select_waveform(size_t wf)
{
	size_t wt_width, wt_height;
	wavetable->get_size(wt_width, wt_height);
	if (wf >= wt_height)
	{
		wf = wt_height - 1;
	}
	selected = wf;

	if (callback)
	{
		callback->on_select(this, selected);
	}
	getParentWidget()->repaint();
}

void WaveformSelector::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();
	float radius = theme->corner_radius;
	float line_width = theme->line_thin;
	float drawable_width = getWidth();
	if (wavetable)
	{
		size_t w, h;

		wavetable->get_size(w, h);
		float box_height = WAVEFORM_BOX_HEIGHT;

		std::vector<Point<float>> line(drawable_width);

		for (size_t pos = 0; pos < h; pos++)
		{
			if (pos == dragging)
			{
				continue;
			}

			Corner corners = pos == 0
							   ? Corner::TOP
							   : (pos == h - 1 ? Corner::BOTTOM : Corner::NONE);
			if (selected == pos)
			{
				theme->foreground.setFor(context);
				fill_rounded_box(context,
								 0,
								 pos * box_height,
								 getWidth(),
								 box_height,
								 theme->corner_radius,
								 theme->line_thin,
								 corners);

				theme->highlight.setFor(context);
				draw_rounded_box(context,
								 0,
								 pos * box_height,
								 getWidth(),
								 box_height,
								 theme->corner_radius,
								 theme->line_thin,
								 corners);
			}
			else
			{
				theme->background.setFor(context);
				draw_rounded_box(context,
								 0,
								 pos * box_height,
								 getWidth(),
								 box_height,
								 theme->corner_radius,
								 1,
								 corners);
			}

			const bool interpolated =
				wavetable_creator && wavetable_creator->is_interpolated(pos);

			// derived waveforms can't be edited: hatch them and dim the curve
			Color curve_color = theme->highlight;
			if (interpolated)
			{
				Color hatch_color = theme->foreground;
				hatch_color.alpha = 0.6f;
				hatch_color.setFor(context, true);
				fill_hatch(context,
						   1,
						   pos * box_height + 1,
						   getWidth() - 2,
						   box_height - 2,
						   6,
						   1);
				curve_color.alpha = 0.6f;
			}

			float rel_pos = (pos + 0.5f) / (h - 1);
			if (h == 1)
			{
				rel_pos = 0;
			}
			for (size_t smpl = 0; smpl < drawable_width; smpl++)
			{
				float phase = (float) smpl / drawable_width;
				float val = wavetable->sample(rel_pos, phase, false, false);
				val = 1 - val * 0.5 - 0.5;
				line[smpl].setX(smpl);
				line[smpl].setY((pos + val) * box_height);
			}

			curve_color.setFor(context, true);
			draw_line_string(context, line, 1);

			Color(255, 255, 255).setFor(context);
			const float index_text_size =
				scaled_font_size(font_role::LABEL, window->get_ui_scale());
			draw_text_clipped(context,
							  std::to_string(pos + 1).c_str(),
							  theme->font.c_str(),
							  index_text_size,
							  Anchor::LEFT_CENTER,
							  5,
							  ((pos + 0.5f) * box_height),
							  0,
							  pos * box_height,
							  getWidth(),
							  box_height);

			if (interpolated)
			{
				Color tag_color(255, 255, 255);
				tag_color.alpha = 0.7f;
				tag_color.setFor(context, true);
				const float tag_text_size =
					scaled_font_size(font_role::CAPTION, window->get_ui_scale());
				draw_text_clipped(context,
								  "INTERP",
								  theme->font.c_str(),
								  tag_text_size,
								  Anchor::RIGHT_CENTER,
								  getWidth() - box_height * 0.5 - 12,
								  ((pos + 0.5f) * box_height),
								  0,
								  pos * box_height,
								  getWidth(),
								  box_height);
			}
		}

		if (dragging >= 0)
		{
			Line<float> line(0,
							 drop_index * WAVEFORM_BOX_HEIGHT,
							 getWidth(),
							 drop_index * WAVEFORM_BOX_HEIGHT);

			theme->highlight.setFor(context);
			line.draw(context, theme->line_thin);
		}
	}

	update_delete_buttons();
}

bool WaveformSelector::onMouse(const MouseEvent& event)
{
	const bool is_handled = FMpireWidget::onMouse(event);
	if (!is_handled && contains_clipped(event.pos) && event.button == 1
		&& event.press)
	{
		float y = event.pos.getY();
		float box_size = WAVEFORM_BOX_HEIGHT;
		select_waveform(y / box_size);
		dragging = -1;
		last_mouse_pos = event.pos;
		return true;
	}
	if (!event.press)
	{
		if (contains_clipped(event.pos) && dragging >= 0)
		{
			if (callback)
			{
				if (drop_index > selected)
				{
					drop_index--;
				}
				callback->on_waveform_moved(this, selected, drop_index);
				select_waveform(drop_index);
			}
			repaint();
		}
		drag_and_drop = nullptr;
		dragging = -2;
	}
	return is_handled;
}

bool WaveformSelector::onMotion(const MotionEvent& event)
{
	if (dragging == -1)
	{
		float dx = event.pos.getX() - last_mouse_pos.getX();
		float dy = event.pos.getY() - last_mouse_pos.getY();
		if (dx * dx + dy * dy < WAVEFORM_DRAG_THRESHOLD * WAVEFORM_DRAG_THRESHOLD)
		{
			return FMpireWidget::onMotion(event);
		}

		dragging = selected;
		drag_and_drop =
			new WaveformDragAndDrop(getTopLevelWidget(), wavetable, selected);
		drag_and_drop->setSize(getWidth(), WAVEFORM_BOX_HEIGHT);
		drag_and_drop->update_position(event.absolutePos);
	}
	else if (dragging >= 0)
	{
		drop_index = event.pos.getY() / WAVEFORM_BOX_HEIGHT + 0.5;
	}
	return FMpireWidget::onMotion(event);
}

void WaveformSelector::onPositionChanged(const PositionChangedEvent& event)
{
	update_delete_button_positions();
}

void WaveformSelector::onResize(const ResizeEvent& event)
{
	update_delete_button_positions();
}

void WaveformSelector::update_delete_buttons()
{
	if (wavetable == nullptr)
	{
		return;
	}
	size_t wt_width, wt_height;
	wavetable->get_size(wt_width, wt_height);

	if (wt_height == delete_buttons.size())
	{
		return;
	}

	if (wt_height == 1)
	{
		delete_buttons.clear();
		return;
	}

	delete_buttons.resize(wt_height);

	for (size_t i = 0; i < wt_height; i++)
	{
		if (delete_buttons[i] == nullptr)
		{
			delete_buttons[i] = new Button(this);
			delete_buttons[i]->set_text("–");
			delete_buttons[i]->set_callback(this);
		}
	}
	update_delete_button_positions();
}

void WaveformSelector::update_delete_button_positions()
{
	const float box_size = WAVEFORM_BOX_HEIGHT;
	for (size_t i = 0; i < delete_buttons.size(); i++)
	{
		delete_buttons[i]->setAbsolutePos(
			getAbsolutePos()
			+ Point<int>(getWidth() - 5 - box_size * 0.5,
						 box_size * (0.25 + i)));
		delete_buttons[i]->setWidth(box_size * 0.5);
		delete_buttons[i]->setHeight(box_size * 0.5);
		delete_buttons[i]->set_drawing_normal_bg(true);
	}
}

WaveformDragAndDrop::WaveformDragAndDrop(Widget* parent,
										 Wavetable* wt,
										 uint32_t idx) :
	FMpireWidget(parent),
	wavetable(wt),
	waveform_idx(idx)
{
}

WaveformDragAndDrop::~WaveformDragAndDrop() noexcept
{
}

void WaveformDragAndDrop::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();

	clip_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 0);

	theme->background.setFor(context);
	fill_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius);

	size_t drawable_width = getWidth();
	std::vector<Point<float>> line(drawable_width);

	size_t wt_width, wt_height;
	wavetable->get_size(wt_width, wt_height);

	float rel_pos = (waveform_idx + 0.5f) / (wt_height - 1);
	if (wt_height == 1)
	{
		rel_pos = 0;
	}

	for (size_t smpl = 0; smpl < drawable_width; smpl++)
	{
		float phase = (float) smpl / drawable_width;
		float val = wavetable->sample(rel_pos, phase, false, false);
		val = 1 - val * 0.5 - 0.5;
		line[smpl].setX(smpl);
		line[smpl].setY(val * getHeight());
	}

	theme->highlight.setFor(context);
	draw_line_string(context, line, 1);

	Color(255, 255, 255).setFor(context);
	const float index_text_size =
		scaled_font_size(font_role::LABEL, window->get_ui_scale());
	draw_text_clipped(context,
					  std::to_string(waveform_idx + 1).c_str(),
					  theme->font.c_str(),
					  index_text_size,
					  Anchor::LEFT_CENTER,
					  5,
					  (0.5f * getHeight()),
					  0,
					  0,
					  getWidth(),
					  getHeight());

	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 theme->line_thin);
}

bool WaveformDragAndDrop::onMotion(const MotionEvent& event)
{
	update_position(event.absolutePos);

	return false;
}

void WaveformDragAndDrop::update_position(Point<double> absolutePos)
{
	uint32_t window_width = getWindow().getWidth();
	uint32_t window_height = getWindow().getHeight();

	float pos_x = absolutePos.getX() + 5;
	if (pos_x + getWidth() > window_width)
	{
		pos_x = absolutePos.getX() - 5 - getWidth();
	}

	float pos_y =
		std::min<float>(absolutePos.getY(), window_height - getHeight());

	setAbsolutePos(pos_x, pos_y);
}

} // namespace fmpire
