#pragma once

enum e_game_obj_type
{
	e_gt_free_cell,
	e_gt_wall,
	e_gt_box,
	e_gt_modifyer,
	e_gt_bomb,
	e_gt_exit
};

enum e_game_modifier
{
	e_modifier_bomb_amount,
	e_modifier_explosion_power,
	//e_modifier_character_speed,

	e_modifier_unknown
};

enum e_enemy_type
{
	e_enemy_flower,
	e_enemy_devil,

	e_enemies_count
};
