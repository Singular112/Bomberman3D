#include "stdafx.h"
#include "character_base_c.h"

#include "game_level_c.h"

template <class GameMapT>
character_base_c<GameMapT>::character_base_c()
{
	m_angle = 0;
	m_transmision_continues = false;
	m_direction_transmition_value = 0.0f;
}


template <class GameMapT>
character_base_c<GameMapT>::~character_base_c()
{
}


template <class GameMapT>
int character_base_c<GameMapT>::get_cell_index(int cell_x_index, int cell_y_index) const
{
	if (cell_x_index < 0
		|| cell_x_index >= m_game_map_ptr->get_map_x_cells_count()
		|| cell_y_index < 0
		|| cell_y_index >= m_game_map_ptr->get_map_y_cells_count())
	{
		return -1;
	}

	return cell_y_index * m_game_map_ptr->get_map_x_cells_count() + cell_x_index;
}


template <class GameMapT>
int character_base_c<GameMapT>::get_cell_x_index() const
{
	return m_current_cell_x_index;
}


template <class GameMapT>
int character_base_c<GameMapT>::get_cell_y_index() const
{
	return m_current_cell_y_index;
}


template <class GameMapT>
e_character_state character_base_c<GameMapT>::get_state() const
{
	return m_current_state;
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_dead() const
{
	return get_state() == e_character_state::e_cs_death;
}


template <class GameMapT>
float character_base_c<GameMapT>::get_speed() const
{
	return m_current_speed_factor;
}


template <class GameMapT>
map_cell_c* character_base_c<GameMapT>::get_current_cell()
{
	int x_cell_index = (int)floorf(m_x_center_pos / (float)m_width);
	int y_cell_index = (int)floorf(m_y_center_pos / (float)m_width);

	return m_game_map_ptr->get_cell(x_cell_index, y_cell_index);
}


template <class GameMapT>
e_way_direction character_base_c<GameMapT>::get_way_direction(const map_cell_c* cell_from,
	const map_cell_c* cell_to)
{
	auto from_x_index = cell_from->get_cell_x_index();
	auto from_y_index = cell_from->get_cell_y_index();

	auto to_x_index = cell_to->get_cell_x_index();
	auto to_y_index = cell_to->get_cell_y_index();

	if (to_x_index != from_x_index)
	{
		if (to_x_index - from_x_index > 0)
		{
			return e_wd_right;
		}
		else
		{
			return e_wd_left;
		}
	}

	if (to_y_index != from_y_index)
	{
		if (to_y_index - from_y_index > 0)
		{
			return e_wd_forward;
		}
		else
		{
			return e_wd_backward;
		}
	}

	return e_wd_no_direction;
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_collide_with(character_base_c<GameMapT>* other_character)
{
#if 0 // working alg
	bool x_left_check1 = m_x_left_pos < (other_character->m_x_left_pos + other_character->m_width);
	bool x_right_check1 = (m_x_left_pos + m_width) > other_character->m_x_left_pos;

	bool y_top_check1 = (m_y_bottom_pos + m_width) > other_character->m_y_bottom_pos;
	bool y_bottom_check1 = m_y_bottom_pos < (other_character->m_y_bottom_pos + other_character->m_width);

	return (x_left_check1 && x_right_check1) &&
			(y_top_check1 && y_bottom_check1);
#else
	int collision_area = (int)(m_width * 0.8f);	// 80% from dimensions
	int collision_area_half_size = (int)(collision_area * 0.5f);

	rect_s rect1
	{
		(int)(m_x_center_pos - collision_area_half_size),
		(int)(m_y_center_pos - collision_area_half_size),
		collision_area,
		collision_area
	};

	rect_s rect2
	{
		(int)(other_character->m_x_center_pos - collision_area_half_size),
		(int)(other_character->m_y_center_pos - collision_area_half_size),
		collision_area,
		collision_area
	};

	return is_rects_overlapped(rect1, rect2);
#endif
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_collide_with(const map_cell3d_c* cell, e_game_obj_type obj_type,
	float character_area_factor, float cell_area_factor)
{
	if (cell->get_cell_type() != obj_type)
	{
		return false;
	}

#if 1
	auto cell_local_position = cell->get_local_position();
	auto cell_width = cell->get_cell_size();
	auto cell_half_width = cell_width * 0.5f;

	int cell_collision_area = (int)(cell_width * cell_area_factor);
	float cell_half_size_collision_area = cell_collision_area * 0.5f;

	float cell_center_x = cell_local_position.x + cell_half_width;
	float cell_center_y = cell_local_position.z + cell_half_width;

	rect_s rect1
	{
		(int)(cell_center_x - cell_half_size_collision_area),
		(int)(cell_center_y - cell_half_size_collision_area),
		cell_collision_area,
		cell_collision_area
	};

	int character_collision_area = (int)(m_width * character_area_factor);
	float character_half_size_collision_area = character_collision_area * 0.5f;

	rect_s rect2
	{
		(int)(m_x_center_pos - character_half_size_collision_area),
		(int)(m_y_center_pos - character_half_size_collision_area),
		character_collision_area,
		character_collision_area
	};

	return is_rects_overlapped(rect1, rect2);

#else  // working algorithm
	auto cell_local_position = cell->get_local_position();
	auto cell_width = cell->get_cell_size();

	bool x_left_check1 = m_x_left_pos < (cell_local_position.x + cell_width);
	bool x_right_check1 = (m_x_left_pos + m_width) > cell_local_position.x;

	bool y_top_check1 = (m_y_bottom_pos + m_width) > cell_local_position.z;
	bool y_bottom_check1 = m_y_bottom_pos < (cell_local_position.z + cell_width);

	return (x_left_check1 && x_right_check1) &&
		(y_top_check1 && y_bottom_check1);
#endif
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_collide_with(explosion3d_c* explosion)
{
	auto explosion_local_position = explosion->get_local_position();
	static auto explosion_area_width = explosion->get_width();
	auto explosion_half_area_width = explosion_area_width * 0.5f;

#if 0 // working algorithm
	bool x_left_check1 = m_x_left_pos < (explosion_local_position.x + explosion_area_width);
	bool x_right_check1 = (m_x_left_pos + m_width) > explosion_local_position.x;

	bool y_top_check1 = (m_y_bottom_pos + m_width) > explosion_local_position.z;
	bool y_bottom_check1 = m_y_bottom_pos < (explosion_local_position.z + explosion_area_width);

	return (x_left_check1 && x_right_check1) &&
		(y_top_check1 && y_bottom_check1);
#else
	int collision_area = (int)(explosion_area_width * 0.10f);	// 10% from dimensions
	float collision_area_half_size = collision_area * 0.5f;

	rect_s rect1
	{
		(int)(m_x_left_pos),
		(int)(m_y_bottom_pos),
		m_width,
		m_width
	};

	float explosion_center_x = explosion_local_position.x + explosion_half_area_width;
	float explosion_center_y = explosion_local_position.z + explosion_half_area_width;

	rect_s rect2
	{
		(int)(explosion_center_x - collision_area_half_size),
		(int)(explosion_center_y - collision_area_half_size),
		collision_area,
		collision_area
	};

	return is_rects_overlapped(rect1, rect2);
#endif
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_rotation_completed() const
{
	return !m_transmision_continues;
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_death_completed() const
{
	return m_death_state_completed;
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_attack_completed() const
{
	return m_attack_state_completed;
}


template <class GameMapT>
bool character_base_c<GameMapT>::is_win_completed() const
{
	return m_win_state_completed;
}


template <class GameMapT>
void character_base_c<GameMapT>::set_game_map_ptr(GameMapT* game_map_ptr)
{
	m_game_map_ptr = game_map_ptr;

	m_width = m_game_map_ptr->get_cell_width();
	m_height = m_game_map_ptr->get_cell_height();
}


template <class GameMapT>
void character_base_c<GameMapT>::set_game_level_ptr(game_level_c* game_level_ptr)
{
	m_game_level_ptr = game_level_ptr;
}


template <class GameMapT>
void character_base_c<GameMapT>::set_cell_position(int cell_x_index, int cell_y_index)
{
	m_current_cell_x_index = cell_x_index;
	m_current_cell_y_index = cell_y_index;
	m_current_cell_index
		= get_cell_index(m_current_cell_x_index, m_current_cell_y_index);

	set_position((float)(cell_x_index * m_width),
		(float)(cell_y_index * m_width));
}


template <class GameMapT>
void character_base_c<GameMapT>::set_position(float x, float y)
{
	if (x < 0)
	{
		x = 0;
	}

	if (y < 0)
	{
		y = 0;
	}

	if (x > (m_game_map_ptr->get_map_x_cells_count() - 1) * m_width)
	{
		x = (float)((m_game_map_ptr->get_map_x_cells_count() - 1) * m_width);
	}

	if (y > (m_game_map_ptr->get_map_y_cells_count() - 1) * m_width)
	{
		y = (float)((m_game_map_ptr->get_map_y_cells_count() - 1) * m_width);
	}

	m_x_left_pos = x;
	m_y_bottom_pos = y;

	m_x_center_pos = m_x_left_pos + m_width / 2;
	m_y_center_pos = m_y_bottom_pos + m_width / 2;

	m_current_cell_x_index = (int)floor(m_x_center_pos / (float)m_width);
	m_current_cell_y_index = (int)floor(m_y_center_pos / (float)m_width);

	m_current_cell_index
		= m_current_cell_y_index * m_game_map_ptr->get_map_x_cells_count()
		+ m_current_cell_x_index;

	compute_covering_cells();
}


template <class GameMapT>
void character_base_c<GameMapT>::set_color(uint32_t color)
{
	m_color = color;
}


template <class GameMapT>
void character_base_c<GameMapT>::set_state(e_character_state state)
{
	m_current_state = state;
}


template <class GameMapT>
void character_base_c<GameMapT>::set_speed(float speed)
{
	m_current_speed_factor = speed;
}


template <class GameMapT>
void character_base_c<GameMapT>::add_modifier(e_game_modifier modifier_type)
{
	switch (modifier_type)
	{
	case e_modifier_bomb_amount:
		m_current_modifiers_setup.bombs_amount++;
	break;

	case e_modifier_explosion_power:
		m_current_modifiers_setup.explosion_power++;
	break;

	//case e_modifier_character_speed:
	//break;
	}

	m_game_modifiers.emplace_back(modifier_type);
}


template <class GameMapT>
void character_base_c<GameMapT>::set_modifiers(const std::vector<e_game_modifier>& game_modifiers)
{
	m_game_modifiers.clear();

	m_current_modifiers_setup.reset();

	for (const auto& modif : game_modifiers)
	{
		add_modifier(modif);
	}
}


template <class GameMapT>
void character_base_c<GameMapT>::set_direction(e_way_direction new_direction)
{
	if (m_current_way_direction == new_direction)
	{
		m_prev_way_direction = m_current_way_direction;
		return;
	}

	m_prev_way_direction = m_current_way_direction;
	m_current_way_direction = new_direction;

	m_prev_angle = (float)m_angle;

	if (m_current_way_direction == e_wd_left)
	{
		m_new_angle = 270;
	}
	else if (m_current_way_direction == e_wd_right)
	{
		m_new_angle = 90;
	}
	else if (m_current_way_direction == e_wd_forward)
	{
		m_new_angle = 0;
	}
	else if (m_current_way_direction == e_wd_backward)
	{
		m_new_angle = 180;
	}

	m_direction_transmition_value = 0.0f;
	m_transmision_continues = true;
}


template <class GameMapT>
void character_base_c<GameMapT>::rotate(float angle_deg)
{
	m_angle = (int)angle_deg;
}


template <class GameMapT>
void character_base_c<GameMapT>::set_current_input_state(e_input_states state)
{
	m_prev_input_state = m_current_input_state;
	m_current_input_state = state;

	if (m_current_input_state != e_input_states::state_none)
	{
		on_start_movement();
	}
	else
	{
		on_stop_movement();
	}

	// rotate character on state change
	if (m_current_input_state != m_prev_input_state)
	{
		if (m_current_input_state == e_input_states::state_run_left)
			set_direction(e_way_direction::e_wd_left);
		else if (m_current_input_state == e_input_states::state_run_right)
			set_direction(e_way_direction::e_wd_right);
		else if (m_current_input_state == e_input_states::state_run_forward)
			set_direction(e_way_direction::e_wd_forward);
		else if (m_current_input_state == e_input_states::state_run_backward)
			set_direction(e_way_direction::e_wd_backward);
	}
}


template <class GameMapT>
void character_base_c<GameMapT>::move_x(float speed)
{
	m_x_move_value += speed;

	int x_inc = 0;

	if (abs(m_x_move_value) >= 1.0f)
	{
		x_inc = (int)m_x_move_value;

		float ceil_part = (float)x_inc;
		m_x_move_value = /*0.0f*/std::modf(speed, &ceil_part);
	}

	set_position(m_x_left_pos + x_inc, m_y_bottom_pos);
}


template <class GameMapT>
void character_base_c<GameMapT>::move_x_near(float speed)
{
	float player_current_cell_x_coord = (float)(m_current_cell_x_index * m_width);

	float x_dist_diff = player_current_cell_x_coord - m_x_left_pos;

	if (abs(speed) > abs(x_dist_diff))
	{
		speed = x_dist_diff;
	}

	move_x(speed);
}


template <class GameMapT>
void character_base_c<GameMapT>::move_y(float speed)
{
	m_y_move_value += speed;

	int y_inc = 0;

	if (abs(m_y_move_value) >= 1.0f)
	{
		y_inc = (int)m_y_move_value;

		float ceil_part = (float)y_inc;
		m_y_move_value = /*0.0f*/std::modf(speed, &ceil_part);
	}

	set_position(m_x_left_pos, m_y_bottom_pos + y_inc);
}


template <class GameMapT>
void character_base_c<GameMapT>::move_y_near(float speed)
{
	float player_current_cell_y_coord = (float)(m_current_cell_y_index * m_width);

	float y_dist_diff = player_current_cell_y_coord - m_y_bottom_pos;

	if (abs(speed) > abs(y_dist_diff))
	{
		speed = y_dist_diff;
	}

	move_y(speed);
}


template <class GameMapT>
void character_base_c<GameMapT>::disable_movement(bool new_state)
{
	m_movement_disabled = new_state;
}


template <class GameMapT>
void character_base_c<GameMapT>::disable_input(bool new_state)
{
	m_input_disabled = new_state;

	set_current_input_state(state_none);
}


template <class GameMapT>
void character_base_c<GameMapT>::update()
{
	if (m_transmision_continues)
	{
		float ang_diff = m_new_angle - m_prev_angle;

		if (abs(ang_diff) > 180)
		{
			if (ang_diff > 0)
			{
				ang_diff = ang_diff - 360;
			}
			else if (ang_diff < 0)
			{
				ang_diff = 360 + ang_diff;
			}
		}

		m_direction_transmition_value += 0.1f;
		if (m_direction_transmition_value >= 1.0f)
		{
			m_direction_transmition_value = 1.0f;
			m_transmision_continues = false;
		}

		m_angle = (int)floor(m_prev_angle + (ang_diff * m_direction_transmition_value));
		if (m_angle >= 360)
			m_angle = m_angle % 360;
		if (m_angle < 0)
			m_angle = 360 + m_angle;
	}
}


template <class GameMapT>
void character_base_c<GameMapT>::update_contol()
{
	// compute speed
#if 1
	float speed = g_last_frame_duration * m_speed_constant;
#else
	float speed = 4;
	//float speed = 3.336;
#endif

	auto is_cell_available = [this](int cell_x_index, int cell_y_index) -> bool
	{
		if (cell_x_index < 0 ||
			cell_y_index < 0 ||
			cell_x_index >= m_game_map_ptr->get_map_x_cells_count() ||
			cell_y_index >= m_game_map_ptr->get_map_y_cells_count())
		{
			return false;
		}

		// check if we already collide with bomb (cause it happens when player plant the bomb)
		bool is_inside_bomb = is_collide_with
		(
			(map_cell3d_c*)m_game_map_ptr->get_cell(cell_x_index, cell_y_index)
			, e_game_obj_type::e_gt_bomb
		);

		return is_inside_bomb ||
			m_game_map_ptr->is_cell_available(cell_x_index, cell_y_index);
	};

	auto move_x_far = [&](float speed, bool near_cell_available)
	{
		if (near_cell_available)
		{
			// move with anyone speed
			move_x(speed);
		}
		else
		{
			// if near cell is busy we need to truncate speed to fit cell
			move_x_near(speed);
		}
	};

	auto move_y_far = [&](float speed, bool near_cell_available)
	{
		if (near_cell_available)
		{
			// move with anyone speed
			move_y(speed);
		}
		else
		{
			// if near cell is busy we need to truncate speed to fit cell
			move_y_near(speed);
		}
	};

	if (m_current_input_state == e_input_states::state_run_left)
	{
		int left_x_cell = m_player_right_corner_x_cell - 1;

		bool check_top_left = is_cell_available(left_x_cell, m_player_top_corner_y_cell);
		bool check_bottom_left = is_cell_available(left_x_cell, m_player_bottom_corner_y_cell);

		if ((check_top_left && check_bottom_left))
		{
			auto near_cell_index = m_current_cell_x_index - 1;
			bool near_cell_available = is_cell_available(near_cell_index, m_player_top_corner_y_cell);

			move_x_far(-speed, near_cell_available);
		}
		else
		{
			if (check_top_left)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					left_x_cell,
					m_player_bottom_corner_y_cell
				);

				auto cell_y_pos = checking_cell->get_cell_y_index() * m_width;
				auto diff = abs(m_y_bottom_pos - cell_y_pos);
				if (diff > (m_width * 0.5))
					move_y_near(+speed);
			}
			else if (check_bottom_left)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					left_x_cell,
					m_player_top_corner_y_cell
				);

				auto cell_y_pos = checking_cell->get_cell_y_index() * m_width;
				auto diff = abs(m_y_bottom_pos - cell_y_pos);
				if (diff > (m_width * 0.5))
					move_y_near(-speed);
			}
		}
	}

	else if (m_current_input_state == e_input_states::state_run_right)
	{
		int right_x_cell = m_player_left_corner_x_cell + 1;

		bool check_top_right = is_cell_available(right_x_cell, m_player_top_corner_y_cell);
		bool check_bottom_right = is_cell_available(right_x_cell, m_player_bottom_corner_y_cell);

		if (check_top_right && check_bottom_right)
		{
			auto near_cell_index = m_current_cell_x_index + 1;
			bool near_cell_available = is_cell_available(near_cell_index, m_player_top_corner_y_cell);

			move_x_far(+speed, near_cell_available);
		}
		else
		{
			if (check_top_right)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					right_x_cell,
					m_player_bottom_corner_y_cell
				);

				auto cell_y_pos = checking_cell->get_cell_y_index() * m_width;
				auto diff = abs(m_y_bottom_pos - cell_y_pos);
				if (diff > (m_width * 0.5))
					move_y_near(+speed);
			}
			else if (check_bottom_right)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					right_x_cell,
					m_player_top_corner_y_cell
				);

				auto cell_y_pos = checking_cell->get_cell_y_index() * m_width;
				auto diff = abs(m_y_bottom_pos - cell_y_pos);
				if (diff > (m_width * 0.5))
					move_y_near(-speed);
			}
		}
	}

	else if(m_current_input_state == e_input_states::state_run_forward)
	{
		int top_y_cell = m_player_bottom_corner_y_cell + 1;

		if (top_y_cell >= m_game_map_ptr->get_map_y_cells_count())
			top_y_cell = m_game_map_ptr->get_map_y_cells_count() - 1;

		bool check_top_left = is_cell_available(m_player_left_corner_x_cell, top_y_cell);
		bool check_top_right = is_cell_available(m_player_right_corner_x_cell, top_y_cell);

		if (check_top_left && check_top_right)
		{
			auto near_cell_index = m_current_cell_y_index + 1;
			bool near_cell_available = is_cell_available(m_player_left_corner_x_cell, near_cell_index);

			move_y_far(+speed, near_cell_available);
		}
		else
		{
			if (check_top_left)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					m_player_right_corner_x_cell,
					top_y_cell
				);

				auto cell_x_pos = checking_cell->get_cell_x_index() * m_width;
				auto diff = abs(m_x_left_pos - cell_x_pos);
				if (diff > (m_width * 0.5))
					move_x_near(-speed);
			}
			else if (check_top_right)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					m_player_left_corner_x_cell,
					top_y_cell
				);

				auto cell_x_pos = checking_cell->get_cell_x_index() * m_width;
				auto diff = abs(m_x_left_pos - cell_x_pos);
				if (diff > (m_width * 0.5))
					move_x_near(+speed);
			}
		}
	}

	else if (m_current_input_state == e_input_states::state_run_backward)
	{
		int bottom_y_cell = m_player_top_corner_y_cell - 1;

		bool check_bottom_left = is_cell_available(m_player_left_corner_x_cell, bottom_y_cell);
		bool check_bottom_right = is_cell_available(m_player_right_corner_x_cell, bottom_y_cell);

		if (check_bottom_left && check_bottom_right)
		{
			auto near_cell_index = m_current_cell_y_index - 1;
			bool near_cell_available = is_cell_available(m_player_left_corner_x_cell, near_cell_index);

			move_y_far(-speed, near_cell_available);
		}
		else
		{
			if (check_bottom_left)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					m_player_right_corner_x_cell,
					bottom_y_cell
				);

				auto cell_x_pos = checking_cell->get_cell_x_index() * m_width;
				auto diff = abs(m_x_left_pos - cell_x_pos);
				if (diff > (m_width * 0.5))
					move_x_near(-speed);
			}
			else if (check_bottom_right)
			{
				const auto& checking_cell = m_game_map_ptr->get_cell
				(
					m_player_left_corner_x_cell,
					bottom_y_cell
				);

				auto cell_x_pos = checking_cell->get_cell_x_index() * m_width;
				auto diff = abs(m_x_left_pos - cell_x_pos);
				if (diff > (m_width * 0.5))
					move_x_near(+speed);
			}
		}
	}
}


template <class GameMapT>
void character_base_c<GameMapT>::handle_input(input_state_s* state)
{
	if (m_input_disabled)
	{
		return;
	}

	// to prevent movement artefacts - if we continue press button then ignore another movement states
	if (state->states.find(m_current_input_state) == state->states.cend())
	{
		if (state->states.find(state_run_left) != state->states.cend())
		{
			set_current_input_state(state_run_left);
		}
		else if (state->states.find(state_run_right) != state->states.cend())
		{
			set_current_input_state(state_run_right);
		}
		else if (state->states.find(state_run_forward) != state->states.cend())
		{
			set_current_input_state(state_run_forward);
		}
		else if (state->states.find(state_run_backward) != state->states.cend())
		{
			set_current_input_state(state_run_backward);
		}
		else
		{
			set_current_input_state(state_none);
		}
	}

	if (state->actions.find(action_plane_bomb) != state->actions.cend())
	{
		place_game_object(e_game_obj_type::e_gt_bomb);
	}
}


template <class GameMapT>
void character_base_c<GameMapT>::compute_covering_cells()
{
	m_player_left_corner_x_cell = (int)floor(m_x_left_pos / m_width);
	m_player_right_corner_x_cell = (int)floor((m_x_left_pos + m_width - 1) / m_width);

	m_player_top_corner_y_cell = (int)floor((m_y_bottom_pos + m_width - 1) / m_width);
	m_player_bottom_corner_y_cell = (int)floor(m_y_bottom_pos / m_width);

	memset(m_covering_cells, 0, sizeof(m_covering_cells));

	if (m_player_left_corner_x_cell != m_player_right_corner_x_cell)
	{
		// horizontal case with 2 covering cells

		m_covering_cells[0] = m_game_map_ptr->get_cell(m_player_left_corner_x_cell,
			m_player_top_corner_y_cell);
		m_covering_cells[1] = m_game_map_ptr->get_cell(m_player_right_corner_x_cell,
			m_player_top_corner_y_cell);
	}
	else if (m_player_top_corner_y_cell != m_player_bottom_corner_y_cell)
	{
		// vertical case with 2 covering cells

		m_covering_cells[0] = m_game_map_ptr->get_cell(m_player_left_corner_x_cell,
			m_player_top_corner_y_cell);
		m_covering_cells[1] = m_game_map_ptr->get_cell(m_player_left_corner_x_cell,
			m_player_bottom_corner_y_cell);
	}
	else
	{
		// case with 1 covering cell

		m_covering_cells[0] = m_game_map_ptr->get_cell(m_player_left_corner_x_cell,
			m_player_top_corner_y_cell);
	}
}


template <class GameMapT>
void character_base_c<GameMapT>::attack()
{
	m_attack_state_completed = false;
	set_state(e_cs_attack);
}


template <class GameMapT>
void character_base_c<GameMapT>::kill(e_kill_type kill_type)
{
	m_death_type = kill_type;
	m_death_state_completed = false;
	set_state(e_cs_death);
}


template <class GameMapT>
void character_base_c<GameMapT>::enter_win_state()
{
	m_win_state_completed = false;
	set_state(e_cs_win);
}


template <class GameMapT>
void character_base_c<GameMapT>::on_attack_complete()
{
	m_attack_state_completed = true;
}


template <class GameMapT>
void character_base_c<GameMapT>::on_death_complete()
{
	m_death_state_completed = true;
}


template <class GameMapT>
void character_base_c<GameMapT>::on_win_complete()
{
	m_win_state_completed = true;
}

//===================================================================

template character_base_c<game_map2d_c>::character_base_c();
template int character_base_c<game_map2d_c>::get_cell_index(int cell_x_index, int cell_y_index) const;
template int character_base_c<game_map2d_c>::get_cell_x_index() const;
template int character_base_c<game_map2d_c>::get_cell_y_index() const;
template void character_base_c<game_map2d_c>::set_game_map_ptr(game_map2d_c* game_map_ptr);
template void character_base_c<game_map2d_c>::set_cell_position(int cell_x_index, int cell_y_index);
template void character_base_c<game_map2d_c>::set_color(uint32_t color);
template void character_base_c<game_map2d_c>::set_state(e_character_state state);
template void character_base_c<game_map2d_c>::set_speed(float speed);
template void character_base_c<game_map2d_c>::set_direction(e_way_direction new_direction);
template void character_base_c<game_map2d_c>::rotate(float angle_deg);
template void character_base_c<game_map2d_c>::set_current_input_state(e_input_states state);
template void character_base_c<game_map2d_c>::move_x(float val);
template void character_base_c<game_map2d_c>::move_y(float val);
template void character_base_c<game_map2d_c>::update();
template void character_base_c<game_map2d_c>::update_contol();
template void character_base_c<game_map2d_c>::handle_input(input_state_s* state);
template void character_base_c<game_map2d_c>::attack();
template void character_base_c<game_map2d_c>::kill(e_kill_type kill_type);
template void character_base_c<game_map2d_c>::enter_win_state();
template map_cell_c* character_base_c<game_map2d_c>::get_current_cell();
template bool character_base_c<game_map2d_c>::is_collide_with(character_base_c<game_map2d_c>* other_character);
template bool character_base_c<game_map2d_c>::is_collide_with(explosion3d_c* explosion);
template bool character_base_c<game_map2d_c>::is_death_completed() const;
template bool character_base_c<game_map2d_c>::is_attack_completed() const;
template bool character_base_c<game_map2d_c>::is_win_completed() const;
template bool character_base_c<game_map2d_c>::is_rotation_completed() const;
template void character_base_c<game_map2d_c>::compute_covering_cells();

template character_base_c<game_map3d_c>::character_base_c();
template int character_base_c<game_map3d_c>::get_cell_index(int cell_x_index, int cell_y_index) const;
template int character_base_c<game_map3d_c>::get_cell_x_index() const;
template int character_base_c<game_map3d_c>::get_cell_y_index() const;
template void character_base_c<game_map3d_c>::set_game_map_ptr(game_map3d_c* game_map_ptr);
template void character_base_c<game_map3d_c>::set_cell_position(int cell_x_index, int cell_y_index);
template void character_base_c<game_map3d_c>::set_color(uint32_t color);
template void character_base_c<game_map3d_c>::set_state(e_character_state state);
template void character_base_c<game_map3d_c>::set_speed(float speed);
template void character_base_c<game_map3d_c>::set_direction(e_way_direction new_direction);
template void character_base_c<game_map3d_c>::rotate(float angle_deg);
template void character_base_c<game_map3d_c>::set_current_input_state(e_input_states state);
template void character_base_c<game_map3d_c>::move_x(float val);
template void character_base_c<game_map3d_c>::move_y(float val);
template void character_base_c<game_map3d_c>::update();
template void character_base_c<game_map3d_c>::update_contol();
template void character_base_c<game_map3d_c>::handle_input(input_state_s* state);
template void character_base_c<game_map3d_c>::attack();
template void character_base_c<game_map3d_c>::kill(e_kill_type kill_type);
template void character_base_c<game_map3d_c>::enter_win_state();
template map_cell_c* character_base_c<game_map3d_c>::get_current_cell();
template bool character_base_c<game_map3d_c>::is_collide_with(character_base_c<game_map3d_c>* other_character);
template bool character_base_c<game_map3d_c>::is_collide_with(const map_cell3d_c* cell, e_game_obj_type obj_type,
	float character_area_factor, float cell_area_factor);
template bool character_base_c<game_map3d_c>::is_collide_with(explosion3d_c* explosion);
template bool character_base_c<game_map3d_c>::is_death_completed() const;
template bool character_base_c<game_map3d_c>::is_attack_completed() const;
template bool character_base_c<game_map3d_c>::is_win_completed() const;
template bool character_base_c<game_map3d_c>::is_rotation_completed() const;
template void character_base_c<game_map3d_c>::compute_covering_cells();
