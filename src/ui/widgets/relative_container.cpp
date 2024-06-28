#include "relative_container.h"

namespace fmpire
{

RelativeContainer::RelativeContainer(Widget* parent) :
	FMpireWidget(parent)
{
}

RelativeContainer::~RelativeContainer() noexcept
{
}

void RelativeContainer::put(SubWidget* const widget,
							const float x,
							const float y,
							const float w,
							const float h)
{
	put(widget, {x, y}, {w, h});
}

void RelativeContainer::put(SubWidget* widget,
							const Point<float> pos,
							const Size<float> size)
{
	relative_children.push_back({pos, size, widget});
}

void RelativeContainer::onDisplay()
{
}

void RelativeContainer::onPositionChanged(const PositionChangedEvent& event)
{
	update_child_positions();
}

void RelativeContainer::onResize(const ResizeEvent& event)
{
	update_child_positions();
}

void RelativeContainer::remove_invalid_children()
{
	std::list<SubWidget*> children = getChildren();

	for (size_t relative_item = 0; relative_item < relative_children.size();
		 relative_item++)
	{
		RelativeChild& relative_child = relative_children[relative_item];
		bool is_child = false;

		for (SubWidget* child : children)
		{
			if (child == relative_child.widget)
			{
				is_child = true;
				break;
			}
		}

		if (!is_child)
		{
			relative_children.erase(relative_children.begin() + relative_item);
			relative_item--;
		}
	}
}

void RelativeContainer::update_child_positions()
{
	remove_invalid_children();
	for (const RelativeChild& relative_child : relative_children)
	{
		uint x = getWidth() * relative_child.relative_position.getX();
		uint y = getHeight() * relative_child.relative_position.getY();
		x += getAbsoluteX();
		y += getAbsoluteY();
		relative_child.widget->setAbsolutePos(x, y);
		uint w = getWidth() * relative_child.relative_size.getWidth();
		uint h = getHeight() * relative_child.relative_size.getHeight();
		relative_child.widget->setSize(w, h);
	}
}

} // namespace fmpire
