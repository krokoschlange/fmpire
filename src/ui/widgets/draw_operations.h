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

void draw_text(const GraphicsContext& context,
			   const char* text,
			   const char* font,
			   const float size,
			   const Anchor anchor,
			   const float x,
			   const float y,
			   const bool full_center = false);

void get_text_rect(const GraphicsContext& context,
				   const char* text,
				   const char* font,
				   const float size,
				   float& width,
				   float& height);

float fit_text(const GraphicsContext& context,
			   const char* text,
			   const char* font,
			   const float size_x,
			   const float size_y);

void draw_line_string(const GraphicsContext& context,
					  const std::vector<Point<float>>& points,
					  const float line_width);

} // namespace fmpire

#endif // DRAW_OPERATIONS_H_INCLUDED
