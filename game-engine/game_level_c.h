#pragma once

#include "enemy3d_c.h"
#include "bomberman3d_c.h"

#include "cam_view_c.h"

#include "object_bomb3d_c.h"

#include "app_state_c.h"

class game_level_c
	: public app_state_c
{
public:
	enum e_level_state
	{
		e_level_state_running,
		e_level_state_finish_by_mob,
		e_level_state_finish_by_bomb,
		e_level_state_finish_by_timeout,
		e_level_state_finish
	};

private:
	typedef std::unique_ptr<enemy3d_c> enemy_ptr_t;

	enum e_level_substate_finish_by_mob
	{
		e_mobfinish_substate_start,
		e_mobfinish_substate_turn_to_player,
		e_mobfinish_substate_mob_attack,
		e_mobfinish_substate_player_death
	};

public:
	game_level_c();

	~game_level_c();

	void on_enter_state();

	void assign_render_buffer_list(linked_list_s<render_buffer_t>* render_buffer_list);

	void assign_cam_view(cam_view_c* cam_view);

	void set_game_modifiers(const std::vector<e_game_modifier>& modifiers);
	std::vector<e_game_modifier> get_game_modifiers();

	void set_input_mapper();

	bool create(int map_x_cell_count, int map_y_cell_count,
		int map_cell_width, int map_cell_height);

	void place_bomberman_on_map();
	enemy3d_c* spawn_enemy(e_enemy_type enemy_type,
		int map_x_cell, int map_y_cell);
	void place_enemies_on_map();
	void place_objects_on_map();

	void update();

	void fill_renderbuffer(render_buffer_t& render_buffer);

	bool is_finished() const;

	e_level_state get_state() const;

protected:
	bool add_explosion(map_cell3d_c* cell);

	void enable_finish_on_map();
	void disable_finish_on_map();

	//
	void on_bomb_explosion(map_cell3d_c* bomb_cell);

private:
	e_level_state m_current_level_state;
	e_level_substate_finish_by_mob m_current_mobfinish_state;

	std::vector<e_game_modifier> m_game_modifiers;
	std::list<map_cell3d_c*> m_modifiers_cell_list;
	map_cell3d_c* m_exit_cell;

	//
	cam_view_c* m_current_cam_view;

	//
	int m_map_x_cell_count, m_map_y_cell_count;
	int m_map_cell_width, m_map_cell_height;

	linked_list_s<render_buffer_t>* m_render_buffer_list_ptr = nullptr;

	std::unique_ptr<input_mapper_c> m_current_mapper;

	//
	std::unique_ptr<game_map3d_c> m_game_map;

	//
	std::unique_ptr<bomberman3d_c> m_bomberman_character;

	//
	int m_max_mobs_limit_on_map = 7;
	std::list<enemy_ptr_t> m_enemy_list;

	//
	std::vector<explosion3d_c*> m_explosion_objects;

	//
	enemy3d_c* m_finish_state_enemy_ptr = nullptr;

	bool m_disable_bombs_on_map = false;

	bool m_is_finished = false;
};
