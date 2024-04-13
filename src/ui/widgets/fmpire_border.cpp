#include "fmpire_border.h"

#include "utils.h"

namespace fmpire
{

FMpireBorder::FMpireBorder(Widget* parent) :
	SubWidget(parent)
{
}

FMpireBorder::~FMpireBorder()
{
}

void FMpireBorder::onDisplay()
{
	const GraphicsContext& context = getGraphicsContext();
	theme->highlight.setFor(context);
	float line_width = 2;
	draw_rounded_box(context, 0, 0, getWidth(), getHeight(), 10, line_width);
}

void FMpireBorder::onPositionChanged(const PositionChangedEvent& event)
{
	getChildren().front()->setAbsolutePos(event.pos);
}

void FMpireBorder::onResize(const ResizeEvent& event)
{
	getChildren().front()->setSize(event.size);
}

}
