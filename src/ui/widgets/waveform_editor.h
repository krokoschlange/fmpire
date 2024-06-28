#ifndef WAVEFORM_EDITOR_H_INCLUDED
#define WAVEFORM_EDITOR_H_INCLUDED

#include "relative_container.h"
#include "Widget.hpp"

namespace fmpire
{

class Waveform;
class WaveformPart;

class WaveformEditor : public RelativeContainer
{
public:
	WaveformEditor(Widget* parent);
	virtual ~WaveformEditor() noexcept;

	void set_waveform(Waveform* wf);

protected:
	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;
	void onPositionChanged(const PositionChangedEvent& event) override;
	void onResize(const ResizeEvent& event) override;

private:
	Waveform* waveform;

	WaveformPart* selection;
};


} // namespace fmpire

#endif // WAVEFORM_EDITOR_H_INCLUDED
