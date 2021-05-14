#include "stdafx.h"
#include "enemy2d_c.h"

enemy2d_c::enemy2d_c()
{
	set_speed(1);
	set_state(e_cs_findway);
}


void enemy2d_c::create()
{
	std::string path = app_directory() + "/enemy.png";

	for (int i = 0; i < 360; i++)
	{
		m_sprites[i].load(path);
		m_sprites[i].rotate(-90 + i);
	}
}
