#include "border.h"

#include "draw_operations.h"

namespace fmpire
{

Border::Border(Widget* parent) :
	SubWidget(parent)
{
}

Border::~Border()
{
}

void Border::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();
	theme->highlight.setFor(context);
	float line_width = 2;
	draw_rounded_box(context, 0, 0, getWidth(), getHeight(), 10, line_width);
}

void Border::onPositionChanged(const PositionChangedEvent& event)
{
	getChildren().front()->setAbsolutePos(event.pos);
}

void Border::onResize(const ResizeEvent& event)
{
	getChildren().front()->setSize(event.size);
}

} // namespace fmpire
