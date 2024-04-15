#ifndef FMPIRE_GRID_CONTAINER_H_INCLUDED
#define FMPIRE_GRID_CONTAINER_H_INCLUDED

#include "fmpire_widget.h"
#include "SubWidget.hpp"

#include <vector>

namespace fmpire
{

class GridContainer : public SubWidget, public FMpireWidget
{
public:
	explicit GridContainer(Widget* parent);
	virtual ~GridContainer();

	void add_row(float weight,
				 float pad_front,
				 float pad_back,
				 int min_size,
				 int max_size);
	void add_column(float weight,
					float pad_front,
					float pad_back,
					int min_size,
					int max_size);

	void remove_row(uint row);
	void remove_column(uint column);

	void configure_row(uint row,
					   float weight,
					   float pad_front,
					   float pad_back,
					   int min_size,
					   int max_size);
	void configure_column(uint column,
						  float weight,
						  float pad_front,
						  float pad_back,
						  int min_size,
						  int max_size);

	void put(SubWidget* widget,
			 uint row,
			 uint column,
			 uint row_span = 1,
			 uint column_span = 1);

protected:
	void onDisplay() override;
	void onPositionChanged(const PositionChangedEvent& event) override;
	void onResize(const ResizeEvent& event) override;

private:
	struct GridChild
	{
		uint row;
		uint column;
		uint row_span;
		uint column_span;
		SubWidget* widget;
	};

	struct GridRow
	{
		float weight;
		float pad_front;
		float pad_back;
		int min_size;
		int max_size;
		int allocated;
	};

	std::vector<GridRow> rows;
	std::vector<GridRow> columns;
	std::vector<GridChild> grid;

	GridChild* get_child_data(SubWidget* widget);
	void remove_invalid_children();

	std::vector<uint> distribute_space(std::vector<GridRow>& items, uint space);
};


} // namespace fmpire

#endif // FMPIRE_GRID_CONTAINER_H_INCLUDED
