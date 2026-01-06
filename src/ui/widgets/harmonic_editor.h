#ifndef HARMONIC_EDITOR_H
#define HARMONIC_EDITOR_H

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
		repaint();
	}

	static constexpr float box_width = 20.0f;

protected:
	virtual void onDisplay() override;

	virtual bool onMouse(const MouseEvent& event) override;
	virtual bool onMotion(const MotionEvent& event) override;

private:
	Ref<HarmonicsWaveformPart> part;
};

} // namespace fmpire


#endif // HARMONIC_EDITOR_H
