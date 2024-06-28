#include "label.h"

#include "draw_operations.h"

namespace fmpire
{

Label::Label(Widget* parent) :
	FMpireWidget(parent),
	text_y_scale(0.75)
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

void Label::set_text_y_scale(const float scale)
{
	text_y_scale = scale;
	repaint();
}

void Label::onDisplay()
{
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
	float text_size = getHeight() * text_y_scale;
	text_size = fit_text(context,
						 text.c_str(),
						 theme->font.c_str(),
						 getWidth(),
						 text_size);
	draw_text(context,
			  text.c_str(),
			  theme->font.c_str(),
			  text_size,
			  Anchor::CENTER,
			  getWidth() / 2,
			  getHeight() / 2);
}

} // namespace fmpire
