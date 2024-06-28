#ifndef SQUARE_CONTAINER_H_INCLUDED
#define SQUARE_CONTAINER_H_INCLUDED

#include "SubWidget.hpp"

namespace fmpire
{

class AspectRatioContainer : public SubWidget
{
public:
	AspectRatioContainer(Widget* parent);
	virtual ~AspectRatioContainer() noexcept;

	void set_aspect_ratio(const float ratio);

protected:
	void onDisplay() override;
	void onPositionChanged(const PositionChangedEvent& event) override;
	void onResize(const ResizeEvent& event) override;

private:
	float aspect_ratio;
};


} // namespace fmpire

#endif // SQUARE_CONTAINER_H_INCLUDED
