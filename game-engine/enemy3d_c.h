#pragma once

#include "character3d_c.h"

class enemy3d_c
	: public character3d_c
{
public:
	enemy3d_c();

	void update();
};

class flower_enemy3d_c
	: public enemy3d_c
{
public:
	void create();
};

class devil_enemy3d_c
	: public enemy3d_c
{
public:
	void create();
};

class frog_enemy3d_c
	: public enemy3d_c
{
public:
	void create();
};
