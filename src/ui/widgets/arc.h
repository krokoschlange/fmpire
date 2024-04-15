#ifndef FMPIRE_ARC_H_INCLUDED
# define FMPIRE_ARC_H_INCLUDED

# include "Geometry.hpp"

# include <vector>

namespace fmpire
{

template<typename T> class Arc
{
public:
	Arc() noexcept;
	Arc(const T& x,
		const T& y,
		const float radius,
		const float start,
		const float end,
		const uint segments = 300);
	Arc(const Point<T>& pos,
		const float radius,
		const float start,
		const float end,
		const uint segments = 300);
	Arc(const Arc<T>& arc) noexcept;

	const T& get_x() const noexcept;

	const T& get_y() const noexcept;

	const Point<T>& get_pos() const noexcept;

	void set_x(const T& x) noexcept;

	void set_y(const T& y) noexcept;

	void set_pos(const T& x, const T& y) noexcept;

	void set_pos(const Point<T>& pos) noexcept;

	const float get_radius() const noexcept;

	void set_radius(const float radius) noexcept;

	const float get_start() const noexcept;

	void set_start(const float start) noexcept;

	const float get_end() const noexcept;

	void set_end(const float end) noexcept;

	const uint get_segments() const noexcept;

	void set_segments(const uint segments);

	void draw(const GraphicsContext& context, T width = 1);

	Arc<T>& operator=(const Arc<T>& arc) noexcept;
	bool operator==(const Arc<T>& arc) const noexcept;
	bool operator!=(const Arc<T>& arc) const noexcept;

private:
	Point<T> center;
	float arc_radius;
	float start_angle;
	float end_angle;
	uint resolution;
};


} // namespace fmpire

#endif // FMPIRE_ARC_H_INCLUDED
// kate: indent-mode cstyle; indent-width 4; replace-tabs off; tab-width 4;
