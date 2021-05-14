#include "stdafx.h"
#include "game_map_c.h"
#include "game_types.h"

template <typename CellT>
game_map_c<CellT>::game_map_c()
{
}


template <typename CellT>
game_map_c<CellT>::~game_map_c()
{
}


template <typename CellT>
void game_map_c<CellT>::create(int x_cell_count, int y_cell_count,
	int cell_width, int cell_height)
{
	m_cell_x_count = x_cell_count;
	m_cell_y_count = y_cell_count;

	m_cell_width = cell_width;
	m_cell_height = cell_height;

	m_box_cells_count = 0;

	m_cells.resize(m_cell_x_count * m_cell_y_count);

	for (int y = 0; y < m_cell_y_count; ++y)
	{
		int line_shift = y * m_cell_x_count;

		for (int x = 0; x < m_cell_x_count; ++x)
		{
			int cell_index = line_shift + x;
			auto& cell = m_cells[cell_index];

			cell.set_cell_size(cell_width);
			cell.set_cell_index(cell_index);
			cell.set_cell_indexes(x, y);

			if (y % 2 == 1 && x % 2 == 1)
			{
				cell.set_cell_type(e_gt_wall);
			}
			else
			{
				if (rand() % 100 > 90 || (x == 0 && y == 0))
				{
					cell.set_cell_type(e_gt_box);

					m_box_cells_count++;
				}
				else
				{
					cell.set_cell_type(e_gt_free_cell);
				}
			}
		}
	}
}


// get

template <typename CellT>
int game_map_c<CellT>::get_cell_index(int cell_x_index, int cell_y_index) const
{
	return cell_y_index * m_cell_x_count + cell_x_index;
}


template <typename CellT>
CellT* game_map_c<CellT>::operator[](int cell_index)
{
	return &m_cells[cell_index];
}


template <typename CellT>
const CellT* game_map_c<CellT>::operator[](int cell_index) const
{
	return &m_cells[cell_index];
}


template <typename CellT>
CellT* game_map_c<CellT>::get_cell(int cell_index)
{
	return &m_cells[cell_index];
}


template <typename CellT>
const CellT* game_map_c<CellT>::get_cell(int cell_index) const
{
	return &m_cells[cell_index];
}


template <typename CellT>
CellT* game_map_c<CellT>::get_cell(int x_index, int y_index)
{
	return &m_cells[get_cell_index(x_index, y_index)];
}


template <typename CellT>
const CellT* game_map_c<CellT>::get_cell(int x_index, int y_index) const
{
	return &m_cells[get_cell_index(x_index, y_index)];
}


template <typename CellT>
bool game_map_c<CellT>::is_cell_available(int cell_index) const
{
	if (cell_index < 0 || cell_index >= (int)m_cells.size())
		return false;

	return m_cells[cell_index].is_available();
}


template <typename CellT>
bool game_map_c<CellT>::is_cell_free(int cell_index) const
{
	if (cell_index < 0 || cell_index >= (int)m_cells.size())
		return false;

	return m_cells[cell_index].get_cell_type() == e_gt_free_cell;
}


template <typename CellT>
bool game_map_c<CellT>::is_cell_available(int cell_x_index, int cell_y_index) const
{
	if (cell_x_index >= m_cell_x_count ||
		cell_y_index >= m_cell_y_count)
	{
		return false;
	}

	return is_cell_available(get_cell_index(cell_x_index, cell_y_index));
}


template <typename CellT>
bool game_map_c<CellT>::is_cell_free(int cell_x_index, int cell_y_index) const
{
	if (cell_x_index >= m_cell_x_count ||
		cell_y_index >= m_cell_y_count)
	{
		return false;
	}

	return is_cell_free(get_cell_index(cell_x_index, cell_y_index));
}


template <typename CellT>
bool game_map_c<CellT>::is_cell_between(int cell_x_index, int cell_y_index,
	e_game_obj_type cell_type) const
{
	bool top_check = true;
	bool bottom_check = true;
	bool left_check = true;
	bool right_check = true;

	if (cell_y_index == 0)
	{
		top_check = false;
	}
	else
	{
		auto top_cell = get_cell(cell_x_index, cell_y_index - 1);
		top_check = top_cell->get_cell_type() == cell_type;
	}

	if (cell_y_index >= m_cell_y_count - 1)
	{
		bottom_check = false;
	}
	else
	{
		auto bottom_cell = get_cell(cell_x_index, cell_y_index + 1);
		bottom_check = bottom_cell->get_cell_type() == cell_type;
	}

	if (cell_x_index == 0)
	{
		left_check = false;
	}
	else
	{
		auto left_cell = get_cell(cell_x_index - 1, cell_y_index);
		left_check = left_cell->get_cell_type() == cell_type;
	}

	if (cell_x_index >= m_cell_x_count - 1)
	{
		right_check = false;
	}
	else
	{
		auto right_cell = get_cell(cell_x_index + 1, cell_y_index);
		right_check = right_cell->get_cell_type() == cell_type;
	}

	return (top_check && bottom_check) || (left_check && right_check);
}


template <typename CellT>
int game_map_c<CellT>::get_map_x_cells_count() const
{
	return m_cell_x_count;
}


template <typename CellT>
int game_map_c<CellT>::get_map_y_cells_count() const
{
	return m_cell_y_count;
}


template <typename CellT>
int game_map_c<CellT>::get_cell_width() const
{
	return m_cell_width;
}


template <typename CellT>
int game_map_c<CellT>::get_cell_height() const
{
	return m_cell_height;
}


template <typename CellT>
int game_map_c<CellT>::get_box_cell_count() const
{
	return m_box_cells_count;
}

// set

template <typename CellT>
void game_map_c<CellT>::pop_object_from_cell(CellT* cell)
{
	auto obj_type = cell->get_cell_type();

	if (obj_type == e_game_obj_type::e_gt_box)
	{
		m_box_cells_count--;
	}

	cell->pop_object();
}


//===================================================================

template game_map_c<map_cell_c>::game_map_c();
template int game_map_c<map_cell_c>::get_map_x_cells_count() const;
template int game_map_c<map_cell_c>::get_map_y_cells_count() const;
template bool game_map_c<map_cell_c>::is_cell_available(int cell_index) const;
template int game_map_c<map_cell_c>::get_cell_width() const;
template int game_map_c<map_cell_c>::get_cell_height() const;

template game_map_c<map_cell3d_c>::game_map_c();
template int game_map_c<map_cell3d_c>::get_map_x_cells_count() const;
template int game_map_c<map_cell3d_c>::get_map_y_cells_count() const;
template bool game_map_c<map_cell3d_c>::is_cell_available(int cell_index) const;
template int game_map_c<map_cell3d_c>::get_cell_width() const;
template int game_map_c<map_cell3d_c>::get_cell_height() const;
template void game_map_c<map_cell3d_c>::pop_object_from_cell(map_cell3d_c* cell);
