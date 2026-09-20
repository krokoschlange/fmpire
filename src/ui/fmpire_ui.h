#ifndef FMPIRE_UI_H_INCLUDED
#define FMPIRE_UI_H_INCLUDED

#include "DistrhoUI.hpp"
#include "fmpire_window.h"
#include "selector.h"

#include "state_manager.h"

USE_NAMESPACE_DISTRHO

namespace fmpire
{
class GridContainer;
class OscillatorPage;
class RelativeContainer;
class SourceListPage;
class WavetableEditor;

class FMpireUI : public UI, public FMpireWindow, public Selector::Callback
{
public:
	FMpireUI();
	virtual ~FMpireUI() noexcept;

	void switch_to_tab(const int tab);

protected:
	void parameterChanged(uint32_t index, float value) override;
	void stateChanged(const char* key, const char* value) override;

	void uiIdle() override;

	void uiFileBrowserSelected(const char* filename) override;

	void onDisplay() override;
	bool onMouse(const MouseEvent& event) override;
	bool onMotion(const MotionEvent& event) override;
	void onResize(const ResizeEvent& ev) override;

	void on_selected(Selector* const selector,
					 const int index,
					 const std::string& option) override;

private:
	StateManager state_manager;
	Ref<GridContainer> grid;
	Ref<RelativeContainer> top_bar;
	Ref<Selector> tab_selector;
	Ref<OscillatorPage> oscillator_page;
	Ref<SourceListPage> mod_page;
	Ref<SourceListPage> fx_page;

	Ref<WavetableEditor> wavetable_editor;
};

} // namespace fmpire

#endif // FMPIRE_UI_H_INCLUDED
