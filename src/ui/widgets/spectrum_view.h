#ifndef SPECTRUM_VIEW_H_INCLUDED
#define SPECTRUM_VIEW_H_INCLUDED

#include "fmpire_widget.h"

namespace fmpire
{
class Waveform;

class SpectrumView : public FMpireWidget
{
public:
	SpectrumView(Widget* parent);
	virtual ~SpectrumView() noexcept;

	void set_waveform(Waveform* wf);

protected:
	void onDisplay();

private:
	Ref<Waveform> waveform;

	std::vector<float> amplitude;
	std::vector<float> phase;
};

} // namespace fmpire

#endif // SPECTRUM_VIEW_H_INCLUDED
