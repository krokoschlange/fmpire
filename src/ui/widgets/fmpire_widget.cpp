#include "fmpire_widget.h"

#include "DistrhoUI.hpp"
#include "fmpire_window.h"
#include "tooltip.h"

namespace fmpire
{

FMpireWidget::FMpireWidget(Widget* parent) :
	SubWidget(parent),
	theme(&ThemeCollection::themes[0]),
	window(dynamic_cast<FMpireWindow*>(getTopLevelWidget()))
{
}

FMpireWidget::~FMpireWidget() noexcept
{
}

void FMpireWidget::show_tooltip(const std::string& text,
								float x,
								float y,
								const bool pin)
{
	Tooltip& tooltip = window->get_tooltip();
	tooltip.request(text, x, y, pin);
}

void FMpireWidget::update_tooltip(const std::string& text)
{
	window->get_tooltip().set_text(text);
}

void FMpireWidget::unpin_tooltip()
{
	window->get_tooltip().unpin();
}

} // namespace fmpire
