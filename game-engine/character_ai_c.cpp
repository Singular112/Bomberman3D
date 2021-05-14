#include "stdafx.h"
#include "character_ai_c.h"

template <class GameMapT>
character_ai_c<GameMapT>::character_ai_c()
{
	m_estimated_way_length = 0;

	m_current_way_direction =
		m_prev_way_direction =
		e_way_direction::e_wd_no_direction;

	m_findway_way_direction =
		m_findway_prev_way_direction =
		e_wd_no_direction;
}


template <class GameMapT>
void character_ai_c<GameMapT>::update()
{
#if 0
	std::string way_dir_str;
	switch (m_current_way_direction)
	{
	case e_way_direction::e_wd_backward:
		way_dir_str = "e_wd_backward";
		break;
	case e_way_direction::e_wd_forward:
		way_dir_str = "e_wd_forward";
		break;
	case e_way_direction::e_wd_right:
		way_dir_str = "e_wd_right";
		break;
	case e_way_direction::e_wd_left:
		way_dir_str = "e_wd_left";
		break;
	case e_way_direction::e_wd_no_direction:
		way_dir_str = "e_wd_no_direction";
		break;
	}
#endif

	character_base_c::update();

	//
	if (m_current_state == e_cs_findway)
	{
		find_way();

		if (m_current_way.size() <= 1)
		{
			return;
		}

		// compute next cell coordinates, set direction & another necessary information
		compute_way_target();

		// start moving
		set_state(e_cs_walk);

		on_start_movement();
	}
	else if (m_current_state == e_cs_walk)
	{
		const auto& next_target_cell = m_current_way[m_next_target_cell_index];

		if (!next_target_cell->is_available())
		{
			// if colliding with an object on map
			set_state(e_cs_findway);
			return;
		}

		// move continues ...

		float x_direction = 0.0f;
		float y_direction = 0.0f;

		if (m_current_way_direction == e_wd_left)
		{
			x_direction = -1.0f;
		}
		else if (m_current_way_direction == e_wd_right)
		{
			x_direction = +1.0f;
		}
		else if (m_current_way_direction == e_wd_forward)
		{
			y_direction = +1.0f;
		}
		else if (m_current_way_direction == e_wd_backward)
		{
			y_direction = -1.0f;
		}

		// compute distance from current position to target cell
		float x_dist_diff = abs(m_x_left_pos - m_move_to_end_x_pos);
		float y_dist_diff = abs(m_y_bottom_pos - m_move_to_end_y_pos);

		// compute speed
		float speed = g_last_frame_duration * m_speed_constant;

		// compute x, y shifts
		float x_shift_speed = x_direction * speed;
		float y_shift_speed = y_direction * speed;

		// check if we outbound from edges & correct it to fit cell
		if (abs(x_shift_speed) > x_dist_diff)
		{
			x_shift_speed = x_dist_diff * x_direction;
		}

		if (abs(y_shift_speed) > y_dist_diff)
		{
			y_shift_speed = y_dist_diff * y_direction;
		}

		// make move
		move_x(x_shift_speed);
		move_y(y_shift_speed);

		// if next cell was reached
		if ((int)m_x_left_pos == m_move_to_end_x_pos
			&& (int)m_y_bottom_pos == m_move_to_end_y_pos)
		{
			m_estimated_way_length--;
			if (m_estimated_way_length == 0)
			{
				// reach end of path
				set_state(e_cs_findway);
				return;
			}

			// compute next cell coordinates, set direction & another necessary information
			compute_way_target();
		}
	}
}


template <class GameMapT>
void character_ai_c<GameMapT>::find_way()
{
	// TODO: Чтобы избежать зацикливания - сделать подсчет повторов направлений.
	// если повторов больше 2-х, то принудительно менять направление на ближайшей свободной клетке!

	// clear old way
	m_current_way.clear();

	// 1. looking for nearest awailable cell. find busy cells
	auto left_cell_index = get_cell_index(m_current_cell_x_index - 1, m_current_cell_y_index);
	auto right_cell_index = get_cell_index(m_current_cell_x_index + 1, m_current_cell_y_index);
	auto top_cell_index = get_cell_index(m_current_cell_x_index, m_current_cell_y_index - 1);
	auto bottom_cell_index = get_cell_index(m_current_cell_x_index, m_current_cell_y_index + 1);

	bool is_left_cell_empty = m_game_map_ptr->is_cell_available(left_cell_index);
	bool is_right_cell_empty = m_game_map_ptr->is_cell_available(right_cell_index);
	bool is_top_cell_empty = m_game_map_ptr->is_cell_available(top_cell_index);
	bool is_bottom_cell_empty = m_game_map_ptr->is_cell_available(bottom_cell_index);

	std::vector<map_cell_c*> free_cells;

	if (is_left_cell_empty)
		free_cells.emplace_back(m_game_map_ptr->get_cell(left_cell_index));
	if (is_right_cell_empty)
		free_cells.emplace_back(m_game_map_ptr->get_cell(right_cell_index));
	if (is_top_cell_empty)
		free_cells.emplace_back(m_game_map_ptr->get_cell(top_cell_index));
	if (is_bottom_cell_empty)
		free_cells.emplace_back(m_game_map_ptr->get_cell(bottom_cell_index));

	auto free_cells_count = free_cells.size();
	if (free_cells_count == 0)
	{
		return;
	}

	// 2. select random direction
	int free_cell_random_direction = rand() % free_cells_count;
	const auto& target_direction_cell = free_cells[free_cell_random_direction];

	int x_shift = target_direction_cell->get_cell_x_index()
		- m_current_cell_x_index;

	int y_shift = target_direction_cell->get_cell_y_index()
		- m_current_cell_y_index;

	auto is_horisontal = [](e_way_direction wd) -> bool
	{
		return wd == e_wd_left || wd == e_wd_right;
	};

	auto detect_direction = [&]()
	{
		if (x_shift > 0)
		{
			m_findway_way_direction = e_wd_right;
		}
		else if (x_shift < 0)
		{
			m_findway_way_direction = e_wd_left;
		}
		else if (y_shift > 0)
		{
			m_findway_way_direction = e_wd_backward;
		}
		else if (y_shift < 0)
		{
			m_findway_way_direction = e_wd_forward;
		}
	};

	m_findway_prev_way_direction = m_findway_way_direction;

	detect_direction();

	if (m_findway_prev_way_direction != e_wd_no_direction &&
		is_horisontal(m_findway_way_direction) == is_horisontal(m_findway_prev_way_direction))
	{
		// if previously direction is same as new direction - swap directions with some roll

		if (rand() % 100 > 10)
		{
			std::swap(x_shift, y_shift);
			detect_direction();
		}
	}

	// 3. random way length
#if 0
	int way_length = 4;
#elif 0
	int way_length = 2 + 2 * (rand() % 4);
#else
	int way_length = 1 + (rand() % 7);
#endif

	// 4. construct way
	int start_way_x_cell = m_current_cell_x_index;
	int start_way_y_cell = m_current_cell_y_index;

	// first - place current cell
	m_current_way.emplace_back
	(
		m_game_map_ptr->get_cell(start_way_x_cell, start_way_y_cell)
	);

	// place other way cells
	for (int i = 0; i < way_length; ++i)
	{
		if (x_shift != 0)
		{
			start_way_x_cell += x_shift;
		}
		else if (y_shift != 0)
		{
			start_way_y_cell += y_shift;
		}

		if (start_way_x_cell >= m_game_map_ptr->get_map_x_cells_count() ||
			start_way_y_cell >= m_game_map_ptr->get_map_y_cells_count() ||
			start_way_x_cell < 0 || start_way_y_cell < 0)
		{
			break;
		}

		const auto& next_cell = m_game_map_ptr->get_cell(start_way_x_cell, start_way_y_cell);
		if (!next_cell->is_available())
		{
			break;
		}

		m_current_way.emplace_back(next_cell);

		// don't allow to complete path between blocks
		if (i == way_length - 1)
		{
			bool is_cell_between_wall = m_game_map_ptr->is_cell_between(start_way_x_cell,
				start_way_y_cell, e_gt_wall);

			if (is_cell_between_wall)
				way_length++;
		}
	}

	// ignore first way cell (first cell points to current object cell)
	m_estimated_way_length = m_current_way.size() - 1;
}


template <class GameMapT>
void character_ai_c<GameMapT>::compute_way_target()
{
	int way_len = m_current_way.size() - 1;

	m_current_way_cell_index = way_len - m_estimated_way_length;
	m_next_target_cell_index = m_current_way_cell_index + 1;

	const auto& current_cell = m_current_way[m_current_way_cell_index];
	const auto& next_target_cell = m_current_way[m_next_target_cell_index];

	m_move_to_end_x_pos =
		next_target_cell->get_cell_x_index() *
		next_target_cell->get_cell_size();

	m_move_to_end_y_pos =
		next_target_cell->get_cell_y_index() *
		next_target_cell->get_cell_size();

	auto direction = get_way_direction(current_cell, next_target_cell);
	set_direction(direction);
}

//===================================================================

template character_ai_c<game_map2d_c>::character_ai_c();
template void character_ai_c<game_map2d_c>::update();
template void character_ai_c<game_map2d_c>::find_way();
template void character_ai_c<game_map2d_c>::compute_way_target();

template character_ai_c<game_map3d_c>::character_ai_c();
template void character_ai_c<game_map3d_c>::update();
template void character_ai_c<game_map3d_c>::find_way();
template void character_ai_c<game_map3d_c>::compute_way_target();
