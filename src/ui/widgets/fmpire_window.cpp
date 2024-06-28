#include "fmpire_window.h"

#include "DistrhoUI.hpp"

namespace fmpire
{

FMpireWindow::FMpireWindow(TopLevelWidget* tlw) :
	tooltip(nullptr),
	top_level_widget(tlw)
{
}

FMpireWindow::~FMpireWindow() noexcept
{
	if (tooltip)
	{
		delete tooltip;
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
