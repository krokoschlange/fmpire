#ifndef FMPIRE_BORDER_H_INCLUDED
#define FMPIRE_BORDER_H_INCLUDED


#include "SubWidget.hpp"
#include "fmpire_widget.h"

namespace fmpire
{

class FMpireBorder : public SubWidget, public FMpireWidget
{
public:
	explicit FMpireBorder(Widget* parent);
	virtual ~FMpireBorder();
	
protected:
	void onDisplay() override;
	void onPositionChanged(const PositionChangedEvent& event) override;
	void onResize(const ResizeEvent& event) override;
};
}

#endif // FMPIRE_BORDER_H_INCLUDED
