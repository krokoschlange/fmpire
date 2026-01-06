#include "fmpire_widget.h"

#include "draw_operations.h"
#include "fmpire_ui.h"
#include "fmpire_window.h"
#include "SubWidget.hpp"
#include "tooltip.h"

namespace fmpire
{

FMpireWidget::FMpireWidget(Widget* parent) :
	SubWidget(parent),
	theme(&ThemeCollection::themes[0]),
	window(static_cast<FMpireUI*>(getTopLevelWidget())),
	clip_radius(0)
{
}

FMpireWidget::~FMpireWidget() noexcept
{
}

void FMpireWidget::set_clip_area(Rectangle<float> area,
								 const float cr,
								 const Corner cc)
{
	clip_area = area;
	clip_radius = cr;
	clip_corners = cc;

	std::list<DGL::SubWidget*> children = getChildren();
	for (std::list<DGL::SubWidget*>::iterator it = children.begin();
		 it != children.end();
		 it++)
	{
		((FMpireWidget*) *it)->set_clip_area(area, cr, cc);
	}
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

void FMpireWidget::clip()
{
	if (clip_area == Rectangle<float>())
	{
		return;
	}

	clip_rounded_box(getGraphicsContext(),
					 clip_area.getX() - getAbsoluteX(),
					 clip_area.getY() - getAbsoluteY(),
					 clip_area.getWidth(),
					 clip_area.getHeight(),
					 clip_radius,
					 0);
}

} // namespace fmpire
