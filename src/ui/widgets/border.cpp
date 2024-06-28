#include "border.h"

#include "draw_operations.h"

namespace fmpire
{

Border::Border(Widget* parent) :
	FMpireWidget(parent)
{
}

Border::~Border()
{
}

void Border::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();
	theme->background.setFor(context);
	float line_width = theme->line_thin;
	float radius = theme->corner_radius;
	draw_rounded_box(context,
					 0,
					 0,
					 getWidth(),
					 getHeight(),
					 radius,
					 line_width);
}

void Border::onPositionChanged(const PositionChangedEvent& event)
{
	if (getChildren().size() == 0)
	{
		return;
	}
	getChildren().front()->setAbsolutePos(event.pos);
}

void Border::onResize(const ResizeEvent& event)
{
	if (getChildren().size() == 0)
	{
		return;
	}
	getChildren().front()->setSize(event.size);
}

} // namespace fmpire
