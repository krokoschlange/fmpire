#ifndef IMAGE_BUTTON_H_INCLUDED
#define IMAGE_BUTTON_H_INCLUDED

#include "button.h"

#include "Image.hpp"

namespace fmpire
{

class ImageButton : public Button
{
public:
	ImageButton(Widget* parent);
	virtual ~ImageButton() noexcept;

protected:
	void onDisplay() override;

private:
	Image* disabled_image;
	Image* normal_image;
	Image* hover_image;
	Image* press_image;
};

} // namespace fmpire

#endif // IMAGE_BUTTON_H_INCLUDED
