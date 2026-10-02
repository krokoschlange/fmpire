#ifndef DRAW_OPERATIONS_H_INCLUDED
#define DRAW_OPERATIONS_H_INCLUDED

#include "DistrhoUI.hpp"
#include "enum_ops.h"

namespace fmpire
{

enum class Anchor
{
	TOP_LEFT,
	TOP_CENTER,
	TOP_RIGHT,
	LEFT_CENTER,
	CENTER,
	RIGHT_CENTER,
	BOTTOM_LEFT,
	BOTTOM_CENTER,
	BOTTOM_RIGHT,
};

enum class Corner : uint32_t
{
	NONE = 0,
	TOP_LEFT = 1,
	TOP_RIGHT = 2,
	BOTTOM_LEFT = 4,
	BOTTOM_RIGHT = 8,
	TOP = TOP_LEFT | TOP_RIGHT,
	BOTTOM = BOTTOM_LEFT | BOTTOM_RIGHT,
	LEFT = TOP_LEFT | BOTTOM_LEFT,
	RIGHT = TOP_RIGHT | BOTTOM_RIGHT,
	ALL = TOP | BOTTOM,
};

template<> struct is_bitflag<Corner> : std::true_type
{
};

void draw_rounded_box(const GraphicsContext& context,
					  float left,
					  float top,
					  float width,
					  float height,
					  float radius,
					  float line_width = 1,
					  Corner corners = Corner::ALL);
void fill_rounded_box(const GraphicsContext& context,
					  float left,
					  float top,
					  float width,
					  float height,
					  float radius,
					  float line_width = 1,
					  Corner corners = Corner::ALL);
void clip_rounded_box(const GraphicsContext& context,
					  float left,
					  float top,
					  float width,
					  float height,
					  float radius,
					  float line_width = 1,
					  Corner corners = Corner::ALL);

void draw_text(const GraphicsContext& context,
			   const char* text,
			   const char* font,
			   const float size,
			   const Anchor anchor,
			   const float x,
			   const float y,
			   const bool full_center = false);

// Like draw_text, but clips drawing to the given rectangle first, so text
// that doesn't fit is cut off instead of silently resized.
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
					   const bool full_center = false);

void get_text_rect(const GraphicsContext& context,
				   const char* text,
				   const char* font,
				   const float size,
				   float& width,
				   float& height);

void get_text_rect(const GraphicsContext& context,
				   const char* text,
				   const char* font,
				   const float size,
				   float& width,
				   float& height,
				   float& bearing_x,
				   float& bearing_y);

// Turns a font_role:: constant (calibrated at UI scale 1.0) into the actual
// font size to draw at for the current UI scale.
inline float scaled_font_size(float role_size, float ui_scale)
{
	return role_size * ui_scale;
}

// Strokes diagonal lines with the current source over the given box. Used to
// mark content that is derived and can't be edited directly.
void fill_hatch(const GraphicsContext& context,
				float left,
				float top,
				float width,
				float height,
				float spacing = 6,
				float line_width = 1);

void draw_line_string(const GraphicsContext& context,
					  const std::vector<Point<float>>& points,
					  const float line_width);

} // namespace fmpire

#endif // DRAW_OPERATIONS_H_INCLUDED
