#include "fmpire_window.h"

#include "DistrhoUI.hpp"
#include "fmpire_widget.h"

namespace fmpire
{

FMpireWindow::FMpireWindow(TopLevelWidget* tlw) :
	tooltip(nullptr),
	top_level_widget(tlw),
	focus(nullptr)
{
}

FMpireWindow::~FMpireWindow() noexcept
{
	if (tooltip)
	{
		delete tooltip;
	}
}

void FMpireWindow::set_focus(FMpireWidget* widget)
{
	FMpireWidget* const previous = focus;
	focus = widget;
	if (previous && previous != widget)
	{
		previous->on_focus_lost();
	}
}

Tooltip& FMpireWindow::get_tooltip()
{
	return *tooltip;
}

void FMpireWindow::reinit_tooltip()
{
	if (tooltip)
	{
		delete tooltip;
	}
	tooltip = new Tooltip(top_level_widget);
}

} // namespace fmpire
