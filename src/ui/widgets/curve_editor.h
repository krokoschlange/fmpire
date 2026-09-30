#ifndef CURVE_EDITOR_H_INCLUDED
#define CURVE_EDITOR_H_INCLUDED

#include "curve.h"
#include "fmpire_widget.h"
#include "modulation_model.h"

namespace fmpire
{

class CurveEditor : public FMpireWidget, public ModulationModel::Listener
{
public:
	CurveEditor(Widget* parent, ModulationModel& modulation_model);
	virtual ~CurveEditor() noexcept;

	// 0 disables snapping on that axis
	void set_grid(const uint32_t x, const uint32_t y);

	virtual void on_modulation_changed() override;
	virtual void on_playhead_changed(const size_t id) override;

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;

private:
	ModulationModel& model;

	size_t modulator_id;
	Curve curve;
	bool is_envelope;

	uint32_t grid_x;
	uint32_t grid_y;

	size_t drag_point;
	size_t drag_handle;
	size_t hover_point;
	size_t hover_handle;

	float ui_scale() const;
	float padding() const;
	float point_radius() const;
	float handle_radius() const;


	Point<float> to_pixels(const float x, const float y) const;
	Point<float> from_pixels(const float px, const float py) const;
	float snap(const float value, const uint32_t divisions) const;

	size_t find_point(const float px, const float py) const;
	size_t find_handle(const float px, const float py) const;

	void send();
};

} // namespace fmpire

#endif // CURVE_EDITOR_H_INCLUDED
