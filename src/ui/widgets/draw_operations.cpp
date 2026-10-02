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

void rounded_box_path(cairo_t* const handle,
					  float left,
					  float top,
					  float width,
					  float height,
					  float radius,
					  float line_width,
					  Corner corners)
{
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
}

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
	rounded_box_path(handle,
					 left,
					 top,
					 width,
					 height,
					 radius,
					 line_width,
					 corners);
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

	rounded_box_path(handle,
					 left,
					 top,
					 width,
					 height,
					 radius,
					 line_width,
					 corners);
	cairo_close_path(handle);
	cairo_fill(handle);
}

void clip_rounded_box(const GraphicsContext& context,
					  float left,
					  float top,
					  float width,
					  float height,
					  float radius,
					  float line_width,
					  Corner corners)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	rounded_box_path(handle,
					 left,
					 top,
					 width,
					 height,
					 radius,
					 line_width,
					 corners);
	cairo_close_path(handle);
	cairo_clip(handle);
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
	cairo_font_extents_t font_xtents;
	cairo_font_extents(handle, &font_xtents);

	int vertical = 1;
	float x_offset = 0;
	switch (anchor)
	{
	case Anchor::TOP_LEFT:
		x_offset = 0;
		vertical = 0;
		break;
	case Anchor::TOP_CENTER:
		x_offset = -xtents.x_advance / 2;
		vertical = 0;
		break;
	case Anchor::TOP_RIGHT:
		x_offset = -xtents.x_advance;
		vertical = 0;
		break;
	case Anchor::LEFT_CENTER:
		x_offset = 0;
		vertical = 1;
		break;
	case Anchor::CENTER:
		x_offset = -xtents.x_advance / 2;
		vertical = 1;
		break;
	case Anchor::RIGHT_CENTER:
		x_offset = -xtents.x_advance;
		vertical = 1;
		break;
	case Anchor::BOTTOM_LEFT:
		x_offset = 0;
		vertical = 2;
		break;
	case Anchor::BOTTOM_CENTER:
		x_offset = -xtents.x_advance / 2;
		vertical = 2;
		break;
	case Anchor::BOTTOM_RIGHT:
		x_offset = -xtents.x_advance;
		vertical = 2;
		break;
	default:
		break;
	}

	float y_offset = 0;
	if (full_center)
	{
		y_offset = -xtents.y_bearing - vertical * xtents.height / 2;
	}
	else
	{
		y_offset = font_xtents.ascent
				   - vertical * (font_xtents.ascent + font_xtents.descent) / 2;
	}
	cairo_move_to(handle, x + x_offset, y + y_offset);
	cairo_show_text(handle, text);
}

void draw_text_clipped(const GraphicsContext& context,
					   const char* text,
					   const char* font,
					   const float size,
					   const Anchor anchor,
					   const float x,
					   const float y,
					   const float clip_left,
					   const float clip_top,
					   const float clip_width,
					   const float clip_height,
					   const bool full_center)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	cairo_save(handle);
	cairo_new_path(handle);
	cairo_rectangle(handle, clip_left, clip_top, clip_width, clip_height);
	cairo_clip(handle);
	draw_text(context, text, font, size, anchor, x, y, full_center);
	cairo_restore(handle);
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

void get_text_rect(const GraphicsContext& context,
				   const char* text,
				   const char* font,
				   const float size,
				   float& width,
				   float& height,
				   float& bearing_x,
				   float& bearing_y)
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
	bearing_x = xtents.x_bearing;
	bearing_y = xtents.y_bearing;
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

void fill_hatch(const GraphicsContext& context,
				float left,
				float top,
				float width,
				float height,
				float spacing,
				float line_width)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	cairo_save(handle);
	cairo_new_path(handle);
	cairo_rectangle(handle, left, top, width, height);
	cairo_clip(handle);

	cairo_new_path(handle);
	for (float offset = -height; offset < width; offset += spacing)
	{
		cairo_move_to(handle, left + offset, top + height);
		cairo_line_to(handle, left + offset + height, top);
	}
	cairo_set_line_width(handle, line_width);
	cairo_stroke(handle);
	cairo_restore(handle);
}

#endif


} // namespace fmpire
