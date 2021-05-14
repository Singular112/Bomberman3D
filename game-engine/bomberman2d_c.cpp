#include "stdafx.h"
#include "bomberman2d_c.h"

void bomberman2d_c::create()
{
	std::string path = app_directory() + "/bomberman.png";

	for (int i = 0; i < 360; i++)
	{
		m_sprites[i].load(path);
		m_sprites[i].rotate(-90 + i);
	}
}


void bomberman2d_c::update()
{
	character_base_c::update();
	update_contol();
}


void bomberman2d_c::place_game_object(e_game_obj_type type)
{
}
