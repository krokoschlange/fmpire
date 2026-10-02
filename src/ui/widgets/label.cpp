#include "label.h"

#include "draw_operations.h"
#include "fmpire_window.h"

namespace fmpire
{

Label::Label(Widget* parent) :
	FMpireWidget(parent)
{
}

Label::~Label() noexcept
{
}

void Label::set_text(std::string txt)
{
	text = txt;
	repaint();
}

void Label::onDisplay()
{
	clip();

	const GraphicsContext& context = getGraphicsContext();

	float line_width = theme->line_thin;
	theme->foreground.setFor(context);
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 line_width);
	Color(255, 255, 255).setFor(context);
	const float text_size = scaled_font_size(font_role::TITLE, window->get_ui_scale());
	draw_text_clipped(context,
					  text.c_str(),
					  theme->font.c_str(),
					  text_size,
					  Anchor::CENTER,
					  getWidth() / 2,
					  getHeight() / 2,
					  0,
					  0,
					  getWidth(),
					  getHeight());
}

} // namespace fmpire
