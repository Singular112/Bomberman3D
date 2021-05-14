#pragma once

#include "character_base_c.h"

template <class GameMapT>
class character_ai_c
	: public character_base_c<GameMapT>
{
public:
	character_ai_c();

	virtual void update();

protected:
	virtual void find_way();

	virtual void compute_way_target();

private:
	const float m_speed_constant = 0.1f;

	// individual variables for path find algorithm,
	// to prevent same movements in same directions on axises
	e_way_direction m_findway_way_direction,
		m_findway_prev_way_direction;

	// AI: path finding
	int m_estimated_way_length = 0;
	std::vector<map_cell_c*> m_current_way;
	int m_move_to_x_cell_index, m_move_to_y_cell_index;
	int m_move_to_end_x_pos, m_move_to_end_y_pos;
};
