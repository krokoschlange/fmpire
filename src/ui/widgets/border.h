#ifndef FMPIRE_BORDER_H_INCLUDED
#define FMPIRE_BORDER_H_INCLUDED


#include "fmpire_widget.h"

namespace fmpire
{

class Border : public FMpireWidget
{
public:
	explicit Border(Widget* parent);
	virtual ~Border();

protected:
	void onDisplay() override;
	void onPositionChanged(const PositionChangedEvent& event) override;
	void onResize(const ResizeEvent& event) override;
};
} // namespace fmpire

#endif // FMPIRE_BORDER_H_INCLUDED
