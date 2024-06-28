#ifndef RELATIVE_CONTAINER_H_INCLUDED
#define RELATIVE_CONTAINER_H_INCLUDED

#include "fmpire_widget.h"

#include <vector>

namespace fmpire
{

class RelativeContainer : public FMpireWidget
{
public:
	explicit RelativeContainer(Widget* parent);
	virtual ~RelativeContainer() noexcept;

	void put(SubWidget* const widget,
			 const float x,
			 const float y,
			 const float w,
			 const float h);
	void put(SubWidget* const widget,
			 const Point<float> pos,
			 const Size<float> size);

protected:
	virtual void onDisplay() override;
	virtual void onPositionChanged(const PositionChangedEvent& event) override;
	virtual void onResize(const ResizeEvent& event) override;

private:
	struct RelativeChild
	{
		Point<float> relative_position;
		Size<float> relative_size;
		SubWidget* widget;
	};

	std::vector<RelativeChild> relative_children;

	void remove_invalid_children();
	void update_child_positions();
};


} // namespace fmpire

#endif // RELATIVE_CONTAINER_H_INCLUDED
