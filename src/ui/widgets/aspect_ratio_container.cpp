#include "aspect_ratio_container.h"

namespace fmpire
{

AspectRatioContainer::AspectRatioContainer(Widget* parent) :
	SubWidget(parent),
	aspect_ratio(1)
{
}

AspectRatioContainer::~AspectRatioContainer() noexcept
{
}

void AspectRatioContainer::onDisplay()
{
}

void AspectRatioContainer::set_aspect_ratio(const float ratio)
{
	aspect_ratio = ratio;
	PositionChangedEvent pos_event;
	pos_event.pos = getAbsolutePos();
	pos_event.oldPos = getAbsolutePos();
	onPositionChanged(pos_event);
	ResizeEvent res_event;
	res_event.size = getSize();
	res_event.oldSize = getSize();
	onResize(res_event);
	repaint();
}

void AspectRatioContainer::onPositionChanged(const PositionChangedEvent& event)
{
	for (SubWidget* child : getChildren())
	{
		child->setAbsolutePos(event.pos);
	}
}

void AspectRatioContainer::onResize(const ResizeEvent& event)
{
	float h = event.size.getHeight();
	float w = h * aspect_ratio;
	if (w > event.size.getWidth())
	{
		w = event.size.getWidth();
		h = w / aspect_ratio;
	}
	for (SubWidget* child : getChildren())
	{
		child->setSize(w, h);
	}
}


} // namespace fmpire
