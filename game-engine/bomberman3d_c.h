#pragma once

#include "character3d_c.h"

class bomberman3d_c
	: public character3d_c
{
public:
	virtual void create();

	void update();

	void set_direction(e_way_direction new_direction);

	void set_current_input_state(e_input_states state);

	void place_game_object(e_game_obj_type type);

	int get_explosion_power() const;

	void on_bomb_explosion(map_cell3d_c* bomb_cell);
};
