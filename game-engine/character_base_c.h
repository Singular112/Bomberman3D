#pragma once

#include <math.h>

#include "game_map2d_c.h"
#include "game_map3d_c.h"

#include "input_manager/input_manager_c.h"

#include "helpers.h"

enum e_way_direction
{
	e_wd_no_direction,

	e_wd_forward,
	e_wd_backward,
	e_wd_left,
	e_wd_right,
};

enum e_character_state
{
	e_cs_idle,
	e_cs_walk,
	e_cs_findway,
	e_cs_run_away,
	e_cs_attack,
	e_cs_death,
	e_cs_win
};

class explosion3d_c;

template <class GameMapT>
class character_base_c
{
public:
	enum e_kill_type
	{
		e_kill_by_fire,
		e_kill_by_enemy
	};

protected:
	const float m_speed_constant = 0.2f;
	//const float m_speed_constant = 0.00f;
	const float m_speed_factor_limit = 1.0f;

public:
	character_base_c();

	virtual ~character_base_c();

	// get

	int get_cell_index(int cell_x_index, int cell_y_index) const;

	int get_cell_x_index() const;
	int get_cell_y_index() const;

	virtual e_character_state get_state() const;

	virtual bool is_dead() const;

	virtual float get_speed() const;

	map_cell_c* get_current_cell();

	virtual e_way_direction get_way_direction(const map_cell_c* cell_from,
		const map_cell_c* cell_to);

	// check if current character collide with different objects ...
	bool is_collide_with(character_base_c<GameMapT>* other_character);
	//bool is_collide_with(const map_cell3d_c* cell, e_game_obj_type obj_type);
	bool is_collide_with(const map_cell3d_c* cell, e_game_obj_type obj_type,
		float character_area_factor = 1.0f, float cell_area_factor = 1.0f);
	bool is_collide_with(explosion3d_c* explosion);

	// rotation is stretched out in time, so we need to know when rotation was completed
	bool is_rotation_completed() const;

	// true when death animation issue is completed
	bool is_death_completed() const;

	// true when attack animation issue is completed
	bool is_attack_completed() const;

	// true when win animation issue is completed
	bool is_win_completed() const;

	//set

	virtual void set_game_map_ptr(GameMapT* game_map_ptr);

	virtual void set_game_level_ptr(class game_level_c* game_level_ptr);

	virtual void set_cell_position(int cell_x_index, int cell_y_index);

	virtual void set_position(float x, float y);

	virtual void set_color(uint32_t color);

	virtual void set_state(e_character_state state);

	virtual void set_speed(float speed);

	virtual void add_modifier(e_game_modifier modifier_type);
	virtual void set_modifiers(const std::vector<e_game_modifier>& game_modifiers);

	virtual void set_direction(e_way_direction new_direction);

	virtual void rotate(float angle_deg);

	virtual void set_current_input_state(e_input_states state);

	virtual void move_x(float speed);
	virtual void move_x_near(float speed);

	virtual void move_y(float speed);
	virtual void move_y_near(float speed);

	virtual void disable_movement(bool new_state);

	virtual void disable_input(bool new_state);

	virtual void place_game_object(e_game_obj_type type) {}

	//
	
	virtual void create() = 0;

	virtual void update();

	virtual void update_contol();

	virtual void render() = 0;

	virtual void handle_input(input_state_s* state);

	//

	void attack();
	void kill(e_kill_type kill_type);

	void enter_win_state();

	//

	virtual void on_start_movement() = 0;
	virtual void on_stop_movement() = 0;
	virtual	void on_attack_complete();
	virtual	void on_death_complete();
	virtual	void on_win_complete();

protected:
	// AI
	virtual void find_way() = 0;
	virtual void compute_way_target() = 0;

	virtual void compute_covering_cells();

protected:
	GameMapT* m_game_map_ptr = nullptr;
	class game_level_c* m_game_level_ptr;

	float m_x_move_value = 0.0f, m_y_move_value = 0.0f;

	float m_x_left_pos, m_y_bottom_pos, m_x_center_pos, m_y_center_pos;
	int m_width, m_height;
	int m_player_left_corner_x_cell, m_player_right_corner_x_cell,
		m_player_top_corner_y_cell, m_player_bottom_corner_y_cell;
	map_cell_c* m_covering_cells[2];

	int m_current_cell_index,
		m_current_cell_x_index, m_current_cell_y_index;

	int m_angle = 0;
	int m_color = 0;
	float m_new_angle = 0.0f;
	float m_prev_angle = 0.0f;
	float m_current_speed_factor = m_speed_constant;

	e_way_direction m_current_way_direction, m_prev_way_direction;
	int m_current_way_cell_index, m_next_target_cell_index;

	float m_direction_transmition_value;	// normalized value
	bool m_transmision_continues;

	e_character_state m_current_state = e_character_state::e_cs_idle;
	e_input_states m_current_input_state = e_input_states::state_none;
	e_input_states m_prev_input_state = e_input_states::state_none;

	bool m_input_disabled = false;
	bool m_movement_disabled = false;

	e_kill_type m_death_type;
	bool m_death_state_completed = false;
	bool m_attack_state_completed = false;
	bool m_win_state_completed = false;

	//
	int m_bombs_already_on_map = 0;

	//
	std::vector<e_game_modifier> m_game_modifiers;

	struct modifiers_setup_s
	{
		int bombs_amount = 1;
		int explosion_power = 1;
		float m_speed_factor = 1.0f;

		void reset()
		{
			bombs_amount = 1;
			explosion_power = 1;
			m_speed_factor = 1.0f;
		}
	};

	modifiers_setup_s m_current_modifiers_setup;
};
