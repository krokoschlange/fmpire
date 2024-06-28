#include "grid_container.h"

namespace fmpire
{

GridContainer::GridContainer(Widget* parent) :
	FMpireWidget(parent)
{
}

GridContainer::~GridContainer()
{
}

void GridContainer::add_row(const float weight,
							const float pad_front,
							const float pad_back,
							const int min_size,
							const int max_size)
{
	rows.push_back({weight, pad_front, pad_back, min_size, max_size, 0});
}

void GridContainer::add_column(const float weight,
							   const float pad_front,
							   const float pad_back,
							   const int min_size,
							   const int max_size)
{
	columns.push_back({weight, pad_front, pad_back, min_size, max_size, 0});
}

void GridContainer::remove_row(const uint row)
{
	if (row >= rows.size())
	{
		return;
	}

	rows.erase(rows.begin() + row);
}

void GridContainer::remove_column(const uint column)
{
	if (column >= columns.size())
	{
		return;
	}

	columns.erase(columns.begin() + column);
}

void GridContainer::configure_row(const uint row,
								  const float weight,
								  const float pad_front,
								  const float pad_back,
								  const int min_size,
								  const int max_size)
{
	if (row >= rows.size())
	{
		return;
	}

	rows[row].weight = weight;
	rows[row].pad_front = pad_front;
	rows[row].pad_back = pad_back;
	rows[row].min_size = min_size;
	rows[row].max_size = max_size;
}

void GridContainer::configure_column(const uint column,
									 const float weight,
									 const float pad_front,
									 const float pad_back,
									 const int min_size,
									 const int max_size)
{
	if (column >= columns.size())
	{
		return;
	}

	columns[column].weight = weight;
	columns[column].pad_front = pad_front;
	columns[column].pad_back = pad_back;
	columns[column].min_size = min_size;
	columns[column].max_size = max_size;
}

void GridContainer::put(SubWidget* widget,
						uint row,
						uint column,
						uint row_span,
						uint column_span)
{
	for (size_t grid_item = 0; grid_item < grid.size(); grid_item++)
	{
		GridChild& grid_child = grid[grid_item];
		if (grid_child.widget == widget)
		{
			grid.erase(grid.begin() + grid_item);
			grid_item--;
		}
	}
	grid.push_back({row, column, row_span, column_span, widget});
}

void GridContainer::onDisplay()
{
}

void GridContainer::onPositionChanged(const PositionChangedEvent& event)
{
	int delta_x = event.pos.getX() - event.oldPos.getX();
	int delta_y = event.pos.getY() - event.oldPos.getY();

	for (GridChild& grid_child : grid)
	{
		grid_child.widget->setAbsoluteX(grid_child.widget->getAbsoluteX()
										+ delta_x);
		grid_child.widget->setAbsoluteY(grid_child.widget->getAbsoluteY()
										+ delta_y);
	}
}

void GridContainer::onResize(const ResizeEvent& event)
{
	remove_invalid_children();
	std::vector<uint> row_borders =
		distribute_space(rows, event.size.getHeight());
	std::vector<uint> column_borders =
		distribute_space(columns, event.size.getWidth());

	for (GridChild& grid_child : grid)
	{
		uint min_x = column_borders[grid_child.column]
				   + columns[grid_child.column].pad_front;
		uint max_x =
			column_borders[grid_child.column + grid_child.column_span]
			- columns[grid_child.column + grid_child.column_span - 1].pad_back;

		uint min_y =
			row_borders[grid_child.row] + rows[grid_child.row].pad_front;
		uint max_y = row_borders[grid_child.row + grid_child.row_span]
				   - rows[grid_child.row + grid_child.row_span - 1].pad_back;

		Point<int> pos = getAbsolutePos();

		min_x += pos.getX();
		max_x += pos.getX();

		min_y += pos.getY();
		max_y += pos.getY();

		grid_child.widget->setAbsolutePos(min_x, min_y);
		grid_child.widget->setSize(max_x - min_x, max_y - min_y);
	}
}

GridContainer::GridChild* GridContainer::get_child_data(SubWidget* widget)
{
	for (GridChild& grid_child : grid)
	{
		if (grid_child.widget == widget)
		{
			return &grid_child;
		}
	}

	return nullptr;
}

void GridContainer::remove_invalid_children()
{
	std::list<SubWidget*> children = getChildren();

	for (size_t grid_item = 0; grid_item < grid.size(); grid_item++)
	{
		GridChild& grid_child = grid[grid_item];
		bool is_child = false;

		for (SubWidget* child : children)
		{
			if (child == grid_child.widget)
			{
				is_child = true;
				break;
			}
		}

		if (!is_child)
		{
			grid.erase(grid.begin() + grid_item);
			grid_item--;
		}
	}
}

std::vector<uint> GridContainer::distribute_space(std::vector<GridRow>& items,
												  uint space)
{
	std::vector<int> allocated(items.size(), 0);
	uint available = space;

	while (available > 0)
	{
		uint selected = items.size();
		float min_param = std::numeric_limits<float>::infinity();

		for (size_t item_idx = 0; item_idx < items.size(); item_idx++)
		{
			if (items[item_idx].max_size > items[item_idx].min_size
				&& allocated[item_idx] >= items[item_idx].max_size)
			{
				continue;
			}

			if (items[item_idx].min_size > allocated[item_idx])
			{
				selected = item_idx;
				break;
			}

			float param = allocated[item_idx] / items[item_idx].weight;

			if (param < min_param)
			{
				min_param = param;
				selected = item_idx;
			}
		}

		// every max size reached, nowhere to put extra space
		if (selected >= items.size())
		{
			break;
		}

		allocated[selected]++;
		available--;
	}

	std::vector<uint> borders(items.size() + 1, 0);

	for (size_t item_idx = 0; item_idx < items.size(); item_idx++)
	{
		borders[item_idx + 1] = borders[item_idx] + allocated[item_idx];
	}

	return borders;
}

} // namespace fmpire


