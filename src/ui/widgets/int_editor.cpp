#include "int_editor.h"

#include "Cairo.hpp"
#include "double_click.h"
#include "draw_operations.h"
#include "fmpire_window.h"

#include <algorithm>
#include <cmath>

namespace fmpire
{

namespace
{
constexpr float LEFT_LABEL_PADDING = 0.15f;
constexpr float MAX_LABEL_PROPORTION = 0.9f;
constexpr double DRAG_THRESHOLD = 3.0;
constexpr size_t MAX_EDIT_DIGITS = 10;
} // namespace

IntEditor::IntEditor(Widget* parent) :
	FMpireWidget(parent),
	value(0),
	default_value(0),
	stored_value(0),
	min_value(0),
	max_value(10),
	dragging(false),
	drag_moved(false),
	scroll_value(0),
	scroll_speed(0.1),
	hover_state(MouseState::NONE),
	press_state(MouseState::NONE),
	editing(false),
	edit_replaces_text(false),
	label_position(LabelPosition::BELOW),
	label_proportion(0),
	left_label_width(0),
	left_label_size(0),
	callback(nullptr)
{
}

IntEditor::~IntEditor() noexcept
{
}

void IntEditor::set_value(const int val, const bool emit_callback)
{
	value = std::clamp(val, min_value, max_value);
	if (callback && emit_callback)
	{
		callback->on_value_changed(this, value);
	}
	repaint();
}

void IntEditor::set_default_value(const int val)
{
	default_value = val;
}

void IntEditor::set_limits(const int min, const int max)
{
	min_value = min;
	max_value = max;
	set_value(value);
}

void IntEditor::set_tooltip(const std::string& text)
{
	tooltip = text;
}

void IntEditor::set_label(const std::string& text)
{
	label = text;
	repaint();
}

void IntEditor::set_label_position(const LabelPosition position,
								   const float proportion)
{
	label_position = position;
	label_proportion = std::clamp(proportion, 0.0f, MAX_LABEL_PROPORTION);
	repaint();
}

void IntEditor::set_callback(Callback* const cb)
{
	callback = cb;
}

IntEditor::MouseState IntEditor::get_zone(const double x) const
{
	const float box_left = get_box_left();
	if (x < box_left)
	{
		return MouseState::NONE;
	}
	const double relative_x = (x - box_left) / (getWidth() - box_left);
	if (relative_x < 0.2)
	{
		return MouseState::REDUCE;
	}
	if (relative_x > 0.8)
	{
		return MouseState::INCREASE;
	}
	return MouseState::CENTER;
}

float IntEditor::get_box_left() const
{
	return label_position == LabelPosition::LEFT ? left_label_width : 0.0f;
}

void IntEditor::update_left_label_metrics(const GraphicsContext& context)
{
	const bool fixed_width = label_proportion > 0;

	if (label_position != LabelPosition::LEFT
		|| (label.empty() && !fixed_width))
	{
		left_label_width = 0;
		left_label_size = 0;
		return;
	}

	const float padding = getHeight() * LEFT_LABEL_PADDING;
	const float max_label_width =
		getWidth() * (fixed_width ? label_proportion : 0.5f);

	left_label_size = scaled_font_size(font_role::LABEL, window->get_ui_scale());
	float text_width = 0, text_height = 0;
	if (!label.empty())
	{
		get_text_rect(context,
					  label.c_str(),
					  theme->font.c_str(),
					  left_label_size,
					  text_width,
					  text_height);
	}

	// don't let the label column eat into the number box; a label that's too
	// wide for max_label_width gets clipped when drawn instead of shrunk
	left_label_width = fixed_width
						  ? max_label_width
						  : std::min(text_width + 2 * padding, max_label_width);
}

void IntEditor::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	update_left_label_metrics(context);

	float radius = theme->corner_radius;
	float line_width = theme->line_thin;
	theme->background.setFor(context);

	const bool label_left = label_position == LabelPosition::LEFT;

	float box_left = get_box_left();
	float box_width = getWidth() - box_left;
	float zone_width = box_width * 0.2;
	float box_top = 0.05 * getHeight();
	float box_bottom = (label_left ? 0.95 : 0.75) * getHeight();
	float box_center = (box_top + box_bottom) / 2;
	float box_height = box_bottom - box_top;

	auto fill_zone = [&](const MouseState zone)
	{
		switch (zone)
		{
		case MouseState::REDUCE:
			fill_rounded_box(context,
							 box_left,
							 box_top,
							 zone_width,
							 box_height,
							 radius,
							 line_width,
							 Corner::LEFT);
			break;
		case MouseState::CENTER:
			fill_rounded_box(context,
							 box_left + zone_width - line_width,
							 box_top,
							 box_width - 2 * zone_width + line_width * 2,
							 box_height,
							 radius,
							 line_width,
							 Corner::NONE);
			break;
		case MouseState::INCREASE:
			fill_rounded_box(context,
							 box_left + box_width - zone_width,
							 box_top,
							 zone_width,
							 box_height,
							 radius,
							 line_width,
							 Corner::RIGHT);
			break;
		default:
			break;
		}
	};

	fill_zone(hover_state);

	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 box_left,
					 box_top,
					 box_width,
					 box_height,
					 radius,
					 line_width);
	draw_rounded_box(context,
					 box_left,
					 box_top,
					 zone_width,
					 box_height,
					 radius,
					 line_width,
					 Corner::LEFT);
	draw_rounded_box(context,
					 box_left + box_width - zone_width,
					 box_top,
					 zone_width,
					 box_height,
					 radius,
					 line_width,
					 Corner::RIGHT);

	theme->foreground.setFor(context);
	fill_zone(press_state);

	float text_center_x = box_left + box_width / 2;

	const std::string text = editing ? edit_text : std::to_string(value);

	float text_size = scaled_font_size(font_role::BODY, window->get_ui_scale());

	if (editing)
	{
		float text_width = 0, text_height = 0;
		if (!text.empty())
		{
			get_text_rect(context,
						  text.c_str(),
						  theme->font.c_str(),
						  text_size,
						  text_width,
						  text_height);
		}

		if (edit_replaces_text)
		{
			Color(1.0f, 1.0f, 1.0f, 0.25f).setFor(context);
			fill_rounded_box(context,
							 text_center_x - text_width / 2 - text_size * 0.15f,
							 box_center - text_size * 0.6f,
							 text_width + text_size * 0.3f,
							 text_size * 1.2f,
							 radius * 0.5f,
							 line_width);
		}
		else
		{
			const float caret_x = text_center_x + text_width / 2 + 1;
			Color(1.0f, 1.0f, 1.0f).setFor(context);
			Line<float> caret(caret_x,
							  box_center - text_size * 0.5f,
							  caret_x,
							  box_center + text_size * 0.5f);
			caret.draw(context, theme->line_thin);
		}
	}

	Color(255, 255, 255).setFor(context);
	draw_text_clipped(context,
					  text.c_str(),
					  theme->font.c_str(),
					  text_size,
					  Anchor::CENTER,
					  text_center_x,
					  box_center,
					  box_left + zone_width,
					  box_top,
					  box_width - 2 * zone_width,
					  box_height,
					  true);
	draw_text_clipped(context,
					  "-",
					  theme->font.c_str(),
					  text_size,
					  Anchor::CENTER,
					  box_left + box_width * 0.1,
					  box_center,
					  box_left,
					  box_top,
					  zone_width,
					  box_height,
					  true);
	draw_text_clipped(context,
					  "+",
					  theme->font.c_str(),
					  text_size,
					  Anchor::CENTER,
					  box_left + box_width * 0.9,
					  box_center,
					  box_left + box_width - zone_width,
					  box_top,
					  zone_width,
					  box_height,
					  true);

	if (label_left)
	{
		draw_text_clipped(context,
						  label.c_str(),
						  theme->font.c_str(),
						  left_label_size,
						  Anchor::LEFT_CENTER,
						  getHeight() * LEFT_LABEL_PADDING,
						  box_center,
						  0,
						  0,
						  left_label_width,
						  getHeight(),
						  true);
	}
	else
	{
		const float below_label_size =
			scaled_font_size(font_role::LABEL, window->get_ui_scale());
		draw_text_clipped(context,
						  label.c_str(),
						  theme->font.c_str(),
						  below_label_size,
						  Anchor::CENTER,
						  getWidth() * 0.5,
						  getHeight() * 0.9,
						  0,
						  0,
						  getWidth(),
						  getHeight(),
						  true);
	}
}

bool IntEditor::onMouse(const MouseEvent& event)
{
	if (event.button == 1 && !event.press)
	{
		press_state = MouseState::NONE;
		repaint();
		if (dragging)
		{
			dragging = false;
			if (!drag_moved && !editing)
			{
				begin_edit();
			}
			return true;
		}
	}
	if (event.button == 1 && event.press && contains_clipped(event.pos))
	{
		const MouseState zone = get_zone(event.pos.getX());
		switch (zone)
		{
		case MouseState::REDUCE:
			commit_edit();
			set_value(value - 1, true);
			press_state = MouseState::REDUCE;
			break;
		case MouseState::INCREASE:
			commit_edit();
			set_value(value + 1, true);
			press_state = MouseState::INCREASE;
			break;
		case MouseState::CENTER:
			if (DoubleClick::is_double_click(event.button, event.time))
			{
				cancel_edit();
				if (value == default_value)
				{
					set_value(stored_value, true);
				}
				else
				{
					stored_value = value;
					set_value(default_value, true);
				}
			}
			else
			{
				dragging = true;
				drag_moved = false;
				press_state = MouseState::CENTER;
				press_mouse_pos = event.pos;
				last_mouse_pos = event.pos;
				scroll_value = value;
			}
			break;
		default:
			return false;
		}
		repaint();
		return true;
	}
	return false;
}

bool IntEditor::onMotion(const MotionEvent& event)
{
	if (contains_clipped(event.pos))
	{
		show_tooltip(tooltip,
					 event.absolutePos.getX(),
					 event.absolutePos.getY(),
					 false);
		hover_state = get_zone(event.pos.getX());
		repaint();
	}
	else
	{
		hover_state = MouseState::NONE;
		repaint();
	}
	if (!dragging)
	{
		return false;
	}

	if (!drag_moved)
	{
		if (std::abs(event.pos.getY() - press_mouse_pos.getY())
			< DRAG_THRESHOLD)
		{
			return true;
		}
		drag_moved = true;
		cancel_edit();
	}

	float diff = last_mouse_pos.getY() - event.pos.getY();

	scroll_value = std::clamp(scroll_value + diff * scroll_speed,
							  static_cast<float>(min_value),
							  static_cast<float>(max_value));
	const int new_value =
		std::clamp<long>(std::lround(scroll_value), min_value, max_value);
	if (new_value != value)
	{
		set_value(new_value, true);
	}

	last_mouse_pos = event.pos;
	return true;
}

bool IntEditor::onCharacterInput(const CharacterInputEvent& event)
{
	if (!editing || window->get_focus() != this)
	{
		return false;
	}

	auto replace_selection = [this]()
	{
		if (edit_replaces_text)
		{
			edit_text.clear();
			edit_replaces_text = false;
		}
	};

	switch (event.character)
	{
	case 0x8: // backspace
		if (edit_replaces_text)
		{
			replace_selection();
		}
		else if (!edit_text.empty())
		{
			edit_text.pop_back();
		}
		break;
	case 0x7F: // delete
	case 0xFF:
		edit_text.clear();
		edit_replaces_text = false;
		break;
	case '-':
		if (min_value < 0)
		{
			replace_selection();
			if (!edit_text.empty() && edit_text[0] == '-')
			{
				edit_text.erase(0, 1);
			}
			else
			{
				edit_text.insert(0, "-");
			}
		}
		break;
	default:
		if (event.character >= '0' && event.character <= '9')
		{
			replace_selection();
			if (edit_text == "0")
			{
				edit_text.clear();
			}
			if (edit_text.size() < MAX_EDIT_DIGITS)
			{
				edit_text.push_back(static_cast<char>(event.character));
			}
		}
		break;
	}

	repaint();
	return true;
}

bool IntEditor::onKeyboard(const KeyboardEvent& event)
{
	if (!editing || window->get_focus() != this || !event.press)
	{
		return false;
	}

	switch (event.key)
	{
	case kKeyEnter:
		commit_edit();
		return true;
	case kKeyEscape:
		cancel_edit();
		return true;
	default:
		return false;
	}
}

void IntEditor::on_focus_lost()
{
	commit_edit();
}

void IntEditor::begin_edit()
{
	editing = true;
	edit_replaces_text = true;
	edit_text = std::to_string(value);
	window->set_focus(this);
	repaint();
}

void IntEditor::commit_edit()
{
	if (!editing)
	{
		return;
	}
	editing = false;
	if (window->get_focus() == this)
	{
		window->set_focus(nullptr);
	}
	repaint();

	if (edit_text.empty() || edit_text == "-")
	{
		return;
	}
	const long long typed = std::stoll(edit_text);
	const int new_value = std::clamp<long long>(typed, min_value, max_value);
	if (new_value != value)
	{
		set_value(new_value, true);
	}
}

void IntEditor::cancel_edit()
{
	if (!editing)
	{
		return;
	}
	editing = false;
	if (window->get_focus() == this)
	{
		window->set_focus(nullptr);
	}
	repaint();
}

} // namespace fmpire
