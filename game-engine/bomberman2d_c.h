#pragma once

#include "character2d_c.h"
#include "game_map2d_c.h"

class bomberman2d_c
	: public character2d_c
{
public:
	void create();

	void update();

	void place_game_object(e_game_obj_type type);

	virtual void on_start_movement() {}
	virtual void on_stop_movement() {}
};
