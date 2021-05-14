#pragma once

#include "character_ai_c.h"
#include "sprite2d_c.h"

class character2d_c
	: public character_ai_c<game_map2d_c>
{
public:
	void render();

protected:
	sprite2d_c m_sprites[360];
};
