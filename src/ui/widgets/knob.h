#ifndef FMPIRE_KNOB_H_INCLUDED
#define FMPIRE_KNOB_H_INCLUDED

#include "DistrhoUI.hpp"
#include "fmpire_widget.h"
#include "modulation_model.h"

#include "theme.h"

namespace fmpire
{

class Knob : public FMpireWidget, public ModulationModel::Listener
{
public:
	explicit Knob(Widget* parentWidget);
	virtual ~Knob() noexcept;
	
	void set_mod_target(ModulationModel& modulation_model,
						const TargetType target,
						const size_t target_object);

	virtual void on_modulation_changed() override;
	virtual void on_route_meter_changed(const size_t slot) override;

	void set_value(float new_value, bool emit_callback = false);
	void set_default_value(const float val);

	void set_label(const std::string& text);
	void set_label_scale(const float scale);
	void set_tooltip(const std::string& txt,
					 const float offset = 0,
					 const float mult = 1,
					 const std::string unit = "");

	struct Callback
	{
	public:
		virtual void drag_started(Knob* const knob) = 0;
		virtual void drag_ended(Knob* const knob) = 0;
		virtual void value_changed(Knob* const knob, const float value) = 0;
	};

	void set_callback(Callback* cb);

protected:
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;

	void onDisplay() override;

private:
	float value;
	float default_value;
	float stored_value;

	std::string label;
	float label_scale;
	std::string tooltip;
	float tooltip_value_offset;
	float tooltip_value_mult;
	std::string tooltip_unit;

	float drag_speed;
	bool dragging;
	Point<double> last_mouse_pos;

	Callback* callback;

	ModulationModel* mod_model;
	TargetType mod_target;
	size_t mod_object;
	bool mod_dragging;
	float mod_amount;

	bool is_mod_mode() const;
	bool on_mod_mouse(const MouseEvent& event);
	bool on_mod_motion(const MotionEvent& event);
	std::string create_mod_tooltip_string() const;
	void draw_mod_range(const GraphicsContext& context,
						const float radius,
						const float down,
						const float up,
						const float width) const;

	std::string create_tooltip_string();
};


} // namespace fmpire

#endif // FMPIRE_KNOB_H_INCLUDED
