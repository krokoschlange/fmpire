#include "utils.h"

namespace fmpire
{

float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

double lerp(double a, double b, double t)
{
	return a + (b - a) * t;
}

void draw_rounded_box(const GraphicsContext& context, float top, float left, float width, float height, float radius, float line_width)
{
	top += line_width / 2;
	left += line_width / 2;
	width -= line_width;
	height -= line_width;
	Arc<float> arc(left + radius, top + radius, radius, 90, 180);
	arc.draw(context, line_width);
	Line<float> line(left + radius, top, left + width - radius, top);
	line.draw(context, line_width);
	arc = Arc<float>(left + width - radius, top + radius, radius, 180, 270);
	arc.draw(context, line_width);
	line = Line<float>(left + width, top + radius, left + width, top + height - radius);
	line.draw(context, line_width);
	arc = Arc<float>(left + width - radius, top + height - radius, radius, 270, 360);
	arc.draw(context, line_width);
	line = Line<float>(left + width - radius, top + height, left + radius, top + height);
	line.draw(context, line_width);
	arc = Arc<float>(left + radius, top + height - radius, radius, 0, 90);
	arc.draw(context, line_width);
	line = Line<float>(left, top + height - radius, left, top + radius);
	line.draw(context, line_width);
}

}
