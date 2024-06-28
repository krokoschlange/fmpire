#include "image_button.h"

#include "draw_operations.h"

namespace fmpire
{
ImageButton::ImageButton(Widget* parent) :
	Button(parent),
	disabled_image(nullptr),
	normal_image(nullptr),
	hover_image(nullptr),
	press_image(nullptr)
{
}

ImageButton::~ImageButton() noexcept
{
}

void ImageButton::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();

	bool draw_bg = false;
	switch (state)
	{
	case State::DISABLED:
	case State::NORMAL:
		break;
	case State::HOVER:
		draw_bg = true;
		theme->foreground.setFor(context);
		break;
	case State::PRESS:
		draw_bg = true;
		theme->highlight.setFor(context);
		break;
	}
	float line_width = theme->line_thin;
	if (draw_bg)
	{
		fill_rounded_box(context,
						 0,
						 0,
						 getWidth(),
						 getHeight(),
						 theme->corner_radius,
						 line_width);
	}

	Image* img = nullptr;
	switch (state)
	{
	case State::DISABLED:
		img = disabled_image;
		break;
	case State::NORMAL:
		img = normal_image;
		break;
	case State::HOVER:
		img = hover_image;
		break;
	case State::PRESS:
		img = press_image;
		break;
	}

	if (img == nullptr && normal_image != nullptr)
	{
		img = normal_image;
	}
	if (img)
	{
		float img_x = getWidth() / 2 - img->getWidth() / 2;
		float img_y = getHeight() / 2 - img->getHeight() / 2;
		img->drawAt(context, img_x, img_y);
	}
	else
	{
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

	switch (state)
	{
	case State::DISABLED:
		theme->background.setFor(context);
		break;
	case State::NORMAL:
		theme->foreground.setFor(context);
		break;
	case State::HOVER:
		theme->highlight.setFor(context);
		break;
	case State::PRESS:
		theme->highlight.setFor(context);
		break;
	}

	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 theme->corner_radius,
					 line_width);
}

} // namespace fmpire
