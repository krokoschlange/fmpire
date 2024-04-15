#ifndef FMPIRE_UI_H_INCLUDED
#define FMPIRE_UI_H_INCLUDED

#include "DistrhoUI.hpp"

#include "grid_container.h"
#include "knob.h"

USE_NAMESPACE_DISTRHO

namespace fmpire
{

class FMpireUI : public UI, public Knob::Callback
{
public:
	FMpireUI();
	virtual ~FMpireUI() noexcept;

protected:
	void parameterChanged(uint32_t index, float value) override;
	void stateChanged(const char* key, const char* value) override;

	void onDisplay() override;
	void onResize(const ResizeEvent& ev) override;

	void drag_started(Knob* const knob) override;
	void drag_ended(Knob* const knob) override;
	void value_changed(Knob* const knob, float value) override;

private:
	ScopedPointer<GridContainer> grid;
};

} // namespace fmpire

START_NAMESPACE_DISTRHO

UI* createUI()
{
	return new fmpire::FMpireUI();
}

END_NAMESPACE_DISTRHO

#endif // FMPIRE_UI_H_INCLUDED
