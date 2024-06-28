#include "draw_operations.h"

#include "arc.h"

#if defined(DGL_CAIRO)
# include "Cairo.hpp"
#endif

#include <string>

namespace fmpire
{

#if defined(DGL_CAIRO)
# include <cairo.h>

void draw_rounded_box(const GraphicsContext& context,
					  float left,
					  float top,
					  float width,
					  float height,
					  float radius,
					  float line_width,
					  Corner corners)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	left += line_width / 2;
	top += line_width / 2;
	width -= line_width;
	height -= line_width;
	cairo_new_sub_path(handle);
	if (any(corners & Corner::TOP_LEFT))
	{
		cairo_arc(handle,
				  left + radius,
				  top + radius,
				  radius,
				  M_PI,
				  3 * M_PI_2);
	}
	else
	{
		cairo_move_to(handle, left, top);
	}
	if (any(corners & Corner::TOP_RIGHT))
	{
		cairo_arc(handle,
				  left + width - radius,
				  top + radius,
				  radius,
				  3 * M_PI_2,
				  2 * M_PI);
	}
	else
	{
		cairo_line_to(handle, left + width, top);
	}
	if (any(corners & Corner::BOTTOM_RIGHT))
	{
		cairo_arc(handle,
				  left + width - radius,
				  top + height - radius,
				  radius,
				  0,
				  M_PI_2);
	}
	else
	{
		cairo_line_to(handle, left + width, top + height);
	}
	if (any(corners & Corner::BOTTOM_LEFT))
	{
		cairo_arc(handle,
				  left + radius,
				  top + height - radius,
				  radius,
				  M_PI_2,
				  M_PI);
	}
	else
	{
		cairo_line_to(handle, left, top + height);
	}
	cairo_close_path(handle);
	cairo_set_line_width(handle, line_width);
	cairo_stroke(handle);
}

void fill_rounded_box(const GraphicsContext& context,
					  float left,
					  float top,
					  float width,
					  float height,
					  float radius,
					  float line_width,
					  Corner corners)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	left += line_width / 2;
	top += line_width / 2;
	width -= line_width;
	height -= line_width;
	cairo_new_sub_path(handle);
	if (any(corners & Corner::TOP_LEFT))
	{
		cairo_arc(handle,
				  left + radius,
				  top + radius,
				  radius,
				  M_PI,
				  3 * M_PI_2);
	}
	else
	{
		cairo_move_to(handle, left, top);
	}
	if (any(corners & Corner::TOP_RIGHT))
	{
		cairo_arc(handle,
				  left + width - radius,
				  top + radius,
				  radius,
				  3 * M_PI_2,
				  2 * M_PI);
	}
	else
	{
		cairo_line_to(handle, left + width, top);
	}
	if (any(corners & Corner::BOTTOM_RIGHT))
	{
		cairo_arc(handle,
				  left + width - radius,
				  top + height - radius,
				  radius,
				  0,
				  M_PI_2);
	}
	else
	{
		cairo_line_to(handle, left + width, top + height);
	}
	if (any(corners & Corner::BOTTOM_LEFT))
	{
		cairo_arc(handle,
				  left + radius,
				  top + height - radius,
				  radius,
				  M_PI_2,
				  M_PI);
	}
	else
	{
		cairo_line_to(handle, left, top + height);
	}
	cairo_close_path(handle);
	cairo_fill(handle);
}

void draw_text(const GraphicsContext& context,
			   const char* text,
			   const char* font,
			   const float size,
			   const Anchor anchor,
			   const float x,
			   const float y,
			   const bool full_center)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	cairo_select_font_face(handle,
						   font,
						   CAIRO_FONT_SLANT_NORMAL,
						   CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(handle, size);
	cairo_text_extents_t xtents;
	cairo_text_extents(handle, text, &xtents);
	float x_offset = 0;
	float y_offset = 0;
	switch (anchor)
	{
	case Anchor::TOP_LEFT:
		x_offset = 0;
		y_offset = xtents.height;
		break;
	case Anchor::TOP_CENTER:
		x_offset = -xtents.x_advance / 2;
		y_offset = xtents.height;
		break;
	case Anchor::TOP_RIGHT:
		x_offset = -xtents.x_advance;
		y_offset = xtents.height;
		break;
	case Anchor::LEFT_CENTER:
		x_offset = 0;
		y_offset = xtents.height / 2;
		break;
	case Anchor::CENTER:
		x_offset = -xtents.x_advance / 2;
		y_offset = xtents.height / 2;
		break;
	case Anchor::RIGHT_CENTER:
		x_offset = -xtents.x_advance;
		y_offset = xtents.height / 2;
		break;
	case Anchor::BOTTOM_LEFT:
		x_offset = 0;
		y_offset = 0;
		break;
	case Anchor::BOTTOM_CENTER:
		x_offset = -xtents.x_advance / 2;
		y_offset = 0;
		break;
	case Anchor::BOTTOM_RIGHT:
		x_offset = -xtents.x_advance;
		y_offset = 0;
		break;
	default:
		break;
	}
	if (full_center)
	{
		y_offset -= (xtents.height + xtents.y_bearing);
	}
	cairo_set_source_rgb(handle, 1, 0, 0);
	/*cairo_rectangle(handle, x + x_offset, y + (xtents.height +
	xtents.y_bearing)- xtents.height + y_offset, xtents.x_advance,
	xtents.height); cairo_fill(handle);*/
	cairo_move_to(handle, x + x_offset, y + y_offset);
	cairo_set_source_rgb(handle, 1, 1, 1);
	cairo_show_text(handle, text);
}

void get_text_rect(const GraphicsContext& context,
				   const char* text,
				   const char* font,
				   const float size,
				   float& width,
				   float& height)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	cairo_select_font_face(handle,
						   font,
						   CAIRO_FONT_SLANT_NORMAL,
						   CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size(handle, size);
	cairo_text_extents_t xtents;
	cairo_text_extents(handle, text, &xtents);
	width = xtents.width;
	height = xtents.height;
}

float fit_text(const GraphicsContext& context,
			   const char* text,
			   const char* font,
			   const float size_x,
			   const float size_y)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	cairo_select_font_face(handle,
						   font,
						   CAIRO_FONT_SLANT_NORMAL,
						   CAIRO_FONT_WEIGHT_NORMAL);
	float font_size = size_y;
	cairo_set_font_size(handle, font_size);
	cairo_text_extents_t xtents;
	cairo_text_extents(handle, text, &xtents);

	font_size =
		std::min<float>(font_size, font_size * size_x / xtents.x_advance);
	font_size = std::min<float>(font_size, font_size * size_y / xtents.height);
	return font_size;
}

void draw_line_string(const GraphicsContext& context,
					  const std::vector<Point<float>>& points,
					  const float line_width)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	cairo_move_to(handle, points[0].getX(), points[0].getY());
	for (size_t point_idx = 1; point_idx < points.size(); point_idx++)
	{
		const Point<float>& point = points[point_idx];
		cairo_line_to(handle, point.getX(), point.getY());
	}
	cairo_set_line_width(handle, line_width);
	cairo_stroke(handle);
}

#endif


} // namespace fmpire
