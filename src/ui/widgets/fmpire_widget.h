#ifndef FMPIRE_WIDGET_H_INCLUDED
#define FMPIRE_WIDGET_H_INCLUDED

#include "fmpire_theme.h"

namespace fmpire
{

class FMpireWidget
{
public:
	FMpireWidget();
	
protected:
	FMpireTheme* theme;
};

}

#endif // FMPIRE_WIDGET_H_INCLUDED
