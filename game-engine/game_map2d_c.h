#pragma once

#include "game_map_c.h"

class game_map2d_c :
	public game_map_c<map_cell_c>
{
	const uint32_t m_green_color = 0x0000FF00;
	const uint32_t m_black_color = 0x00000000;
	const uint32_t m_white_color = 0x00FFFFFF;

public:
	void update() {}

	void render();
};
