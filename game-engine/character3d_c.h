#pragma once

#include "mesh_factory_c.h"

#include "character_ai_c.h"

class character3d_c
	: public character_ai_c<game_map3d_c>
{
	enum e_fire_death_state
	{
		e_death_state_nostate,
		e_death_state_wait,
		e_death_state_ashfall_waiting_for_clipping_start,
		e_death_state_ashfall_started,
		e_death_state_ashfall_finished
	};

public:
	character3d_c();
	~character3d_c();

	void create();

	void create_burnt_material_set();

	void update();
	void update_ashfalls(camera_t* cam_ptr);

	void render() {}

	void fill_renderbuffer();

	void ashfall_fill_renderbuffer(sr::render_buffer_t& render_buffer);

	render_buffer_t* get_render_buffer();

	void on_start_movement();
	void on_stop_movement();

	void attack();
	void kill(e_kill_type kill_type);
	void enter_win_state();

protected:
	game_object_c m_obj;

	// resource copy for nice graphic effects
	enum e_material_set
	{
		e_material_set_base,
		e_material_set_burnt
	};
	mesh_base_c::material_library_t m_materials[2];

	float m_animation_blend_time = 0.1f;

	int m_animation_index_run = 0,
		m_animation_index_idle = 0,
		m_animation_index_death = 0,
		m_animation_index_win = 0,
		m_animation_index_loose = 0,
		m_animation_index_dance = 0,
		m_animation_index_attack = 0;

	int m_animation_attack_repeat_count = 1;

	e_fire_death_state m_fire_death_state;
	std::chrono::time_point<std::chrono::system_clock> m_fire_death_wait_timeout;
	float m_y_clip_plane = 0;
	size_t m_clipped_points_count = 0;
};
