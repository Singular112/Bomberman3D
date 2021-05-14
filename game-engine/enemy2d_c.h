#pragma once

#include "character2d_c.h"

class enemy2d_c
	: public character2d_c
{
public:
	enemy2d_c();

	void create();

	void on_start_movement() {}
	void on_stop_movement() {}

private:
};
