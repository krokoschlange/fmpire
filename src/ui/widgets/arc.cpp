#include "arc.h"

#if defined(DGL_CAIRO)
# include "Cairo.hpp"
#elif defined(DGL_OPENGL)
# include "OpenGL-include.hpp"
#endif

namespace fmpire
{

template<typename T>
Arc<T>::Arc() noexcept :
	center(0, 0),
	arc_radius(0),
	start_angle(0),
	end_angle(0),
	resolution(0)
{
}

template<typename T>
Arc<T>::Arc(const T& x,
			const T& y,
			const float radius,
			const float start,
			const float end,
			const uint segments) :
	center(x, y),
	arc_radius(radius),
	start_angle(start),
	end_angle(end),
	resolution(segments)
{
}

template<typename T>
Arc<T>::Arc(const Point<T>& pos,
			const float radius,
			const float start,
			const float end,
			const uint segments) :
	center(pos),
	arc_radius(radius),
	start_angle(start),
	end_angle(end),
	resolution(segments)
{
}

template<typename T>
Arc<T>::Arc(const Arc<T>& arc) noexcept :
	center(arc.center),
	arc_radius(arc.arc_radius),
	start_angle(arc.start_angle),
	end_angle(arc.end_angle),
	resolution(arc.resolution)
{
}

template<typename T> const T& Arc<T>::get_x() const noexcept
{
	return center.getX();
}

template<typename T> const T& Arc<T>::get_y() const noexcept
{
	return center.getY();
}

template<typename T> const Point<T>& Arc<T>::get_pos() const noexcept
{
	return center;
}

template<typename T> void Arc<T>::set_x(const T& x) noexcept
{
	center.setX(x);
}

template<typename T> void Arc<T>::set_y(const T& y) noexcept
{
	center.setY(y);
}

template<typename T> void Arc<T>::set_pos(const T& x, const T& y) noexcept
{
	center.setPos(x, y);
}

template<typename T> void Arc<T>::set_pos(const Point<T>& pos) noexcept
{
	center = pos;
}

template<typename T> const float Arc<T>::get_radius() const noexcept
{
	return arc_radius;
}

template<typename T> void Arc<T>::set_radius(const float radius) noexcept
{
	arc_radius = radius;
}

template<typename T> const float Arc<T>::get_start() const noexcept
{
	return start_angle;
}

template<typename T> void Arc<T>::set_start(const float start) noexcept
{
	start_angle = start;
}

template<typename T> const float Arc<T>::get_end() const noexcept
{
	return end_angle;
}

template<typename T> void Arc<T>::set_end(const float end) noexcept
{
	end_angle = end;
}

template<typename T> const uint Arc<T>::get_segments() const noexcept
{
	return resolution;
}

template<typename T> void Arc<T>::set_segments(const uint segments)
{
	resolution = segments;
}

#if defined(DGL_CAIRO)
# include <cairo.h>

template<typename T> void Arc<T>::draw(const GraphicsContext& context, T width)
{
	cairo_t* const handle = ((const CairoGraphicsContext&) context).handle;

	double rad_start = start_angle * (M_PI / 180) + M_PI_2;
	double rad_End = end_angle * (M_PI / 180) + M_PI_2;

	cairo_new_sub_path(handle);
	cairo_arc(handle,
			  center.getX(),
			  center.getY(),
			  arc_radius,
			  rad_start,
			  rad_End);
	cairo_set_line_width(handle, width);
	cairo_stroke(handle);
}
#elif defined(DGL_OPENGL)
template<typename T> void Arc<T>::draw(const GraphicsContext& context, T width)
{
	float rad_start = start_angle * (M_PI / 180);
	float rad_end = end_angle * (M_PI / 180);
	float start_sin = std::sin(rad_start);
	float start_cos = std::cos(rad_start);

	float x = -start_sin * arc_radius;
	float y = start_cos * arc_radius;

	float delta_angle = (rad_end - rad_start) / static_cast<float>(resolution);
	float delta_sin = std::sin(delta_angle);
	float delta_cos = std::cos(delta_angle);

	glLineWidth(static_cast<GLfloat>(width));
	glDisable(GL_LINE_SMOOTH);
	glBegin(GL_LINE_STRIP);

	glVertex2d(center.getX() + x, center.getY() + y);

	for (size_t point_idx = 1; point_idx < resolution; point_idx++)
	{
		float old_x = x;
		float old_y = y;
		x = delta_cos * old_x - delta_sin * old_y;
		y = delta_sin * old_x + delta_cos * old_y;

		glVertex2d(center.getX() + x, center.getY() + y);
	}

	float old_x = x;
	float old_y = y;
	x = delta_cos * old_x - delta_sin * old_y;
	y = delta_sin * old_x + delta_cos * old_y;
	glVertex2d(center.getX() + x, center.getY() + y);

	glEnd();
}
#endif

template<typename T> Arc<T>& Arc<T>::operator=(const Arc<T>& arc) noexcept
{
	center = arc.center;
	arc_radius = arc.arc_radius;
	start_angle = arc.start_angle;
	end_angle = arc.end_angle;
	resolution = arc.resolution;
	return *this;
}

template<typename T> bool Arc<T>::operator==(const Arc<T>& arc) const noexcept
{
	return (center == arc.center && d_isEqual(arc_radius, arc.arc_radius)
			&& d_isEqual(start_angle, arc.start_angle)
			&& d_isEqual(end_angle, arc.end_angle)
			&& resolution == arc.resolution);
}

template<typename T> bool Arc<T>::operator!=(const Arc<T>& arc) const noexcept
{
	return (center != arc.center || d_isNotEqual(arc_radius, arc.arc_radius)
			|| d_isNotEqual(start_angle, arc.start_angle)
			|| d_isNotEqual(end_angle, arc.end_angle)
			|| resolution != arc.resolution);
}


template class Arc<double>;
template class Arc<float>;
template class Arc<int>;
template class Arc<uint>;
template class Arc<short>;
template class Arc<ushort>;

} // namespace fmpire
