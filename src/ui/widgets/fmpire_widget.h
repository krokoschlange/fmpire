#ifndef FMPIRE_WIDGET_H_INCLUDED
#define FMPIRE_WIDGET_H_INCLUDED

#include "Geometry.hpp"
#include "SubWidget.hpp"

#include "draw_operations.h"
#include "ref_counted.h"
#include "theme.h"

namespace fmpire
{
class FMpireWindow;

class FMpireWidget : public SubWidget, public RefCounted
{
public:
	FMpireWidget(Widget* parent);
	virtual ~FMpireWidget() noexcept;

	virtual void set_clip_area(Rectangle<float> area,
							   const float cr,
							   const Corner cc);

	inline Rectangle<float> get_clip_area() const { return clip_area; }

	inline float get_clip_radius() const { return clip_radius; }

	inline Corner get_clip_corners() const { return clip_corners; }

	template<typename T> inline bool contains_clipped(const Point<T>& pos) const
	{
		return contains_clipped(pos.getX(), pos.getY());
	}

	template<typename T> inline bool contains_clipped(T x, T y) const
	{
		bool is_in_clip_area =
			clip_area == Rectangle<float>()
				? true
				: clip_area.contains(x + getAbsoluteX(), y + getAbsoluteY());

		return is_in_clip_area && contains(x, y);
	}

protected:
	Theme* theme;

	void show_tooltip(const std::string& text,
					  float x,
					  float y,
					  const bool pin = false);
	void update_tooltip(const std::string& text);
	void unpin_tooltip();

	void clip();

	FMpireWindow* window;

	Rectangle<float> clip_area;
	float clip_radius;
	Corner clip_corners;
};

} // namespace fmpire

#endif // FMPIRE_WIDGET_H_INCLUDED
