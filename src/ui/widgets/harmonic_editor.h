#ifndef HARMONIC_EDITOR_H
#define HARMONIC_EDITOR_H

#include <vector>

#include "fmpire_widget.h"

namespace fmpire
{
class HarmonicsWaveformPart;

class HarmonicEditor : public FMpireWidget
{
public:
	HarmonicEditor(Widget* parent);
	virtual ~HarmonicEditor() noexcept;

	void set_part(HarmonicsWaveformPart* p)
	{
		part = p;
		stored_amplitude.clear();
		stored_phase.clear();
		repaint();
	}

	HarmonicsWaveformPart* get_part() const { return part; }

	struct Callback
	{
		virtual void on_harmonics_edited(HarmonicEditor* const editor,
										 HarmonicsWaveformPart* const part,
										 const bool is_done) = 0;
	};

	void set_callback(Callback* const cb) { callback = cb; }

	static constexpr float box_width = 20.0f;

protected:
	virtual void onDisplay() override;

	virtual bool onMouse(const MouseEvent& event) override;
	virtual bool onMotion(const MotionEvent& event) override;

private:
	void apply_drag_position(const Point<double>& pos);

	Ref<HarmonicsWaveformPart> part;
	Callback* callback;

	bool is_pressed;
	bool dragging_phase;

	std::vector<float> stored_amplitude;
	std::vector<float> stored_phase;
};

} // namespace fmpire


#endif // HARMONIC_EDITOR_H
