#include "fmpire_ui.h"
#include "arc.h"

#include "Color.hpp"

#include "fmpire_border.h"
#include "fmpire_knob.h"

#include <string>

namespace fmpire
{

FMpireUI::FMpireUI()
{
	grid = new FMpireGridContainer(this);
	grid->setAbsolutePos(0, 0);
	grid->setSize(100, 100);
	
	grid->add_row(1, 0, 3, 0, 150);
	grid->add_row(3, 0, 0, 0, 0);
	grid->add_column(1, 0, 0, 0, 0);
	
	FMpireKnob* knob = new FMpireKnob(grid);
	grid->put(knob, 1, 0);
	knob->set_callback(this);
}

FMpireUI::~FMpireUI() noexcept
{
	for (SubWidget* widget : grid->getChildren())
	{
		for (SubWidget* widget2 : widget->getChildren())
		{
			delete widget2;
		}
		delete widget;
	}
}

void FMpireUI::parameterChanged(uint32_t index, float value)
{
}

void FMpireUI::stateChanged(const char* key, const char* value)
{
	d_stdout("state change %s %s", key, value);
}

void FMpireUI::onDisplay()
{
	
}

void FMpireUI::onResize(const ResizeEvent & ev)
{
	grid->setSize(ev.size);
}

void FMpireUI::drag_started(FMpireKnob *const knob) {}
void FMpireUI::drag_ended(FMpireKnob *const knob) {}
void FMpireUI::value_changed(FMpireKnob *const knob, float value) {
	setState("vol", std::to_string(value).c_str());
}


}
