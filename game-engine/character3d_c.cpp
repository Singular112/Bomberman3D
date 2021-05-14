#include "stdafx.h"
#include "character3d_c.h"


character3d_c::character3d_c()
	: m_obj(true)
	, m_fire_death_state(e_death_state_nostate)
{

}


character3d_c::~character3d_c()
{
}


void character3d_c::create()
{
	m_materials[e_material_set_base] = m_obj.get_base_material_library();

	create_burnt_material_set();

	m_obj.use_entity_list_copy(false);

	m_obj.set_polygons_attributes(POLY_ATTR_ZBUFFER);

	m_obj.set_y_angle((float)m_angle);
}


void character3d_c::create_burnt_material_set()
{
	m_materials[e_material_set_burnt] = m_materials[e_material_set_base];

	auto& burnt_material_set = m_materials[e_material_set_burnt];

	for (auto& material : burnt_material_set)
	{
		material.k_diffuse_color.r =
			material.k_diffuse_color.g =
			material.k_diffuse_color.b =
			60;
	}
}


void character3d_c::update()
{
	character_ai_c<game_map3d_c>::update();

	if (m_current_state == e_cs_attack)
	{
		if (m_obj.is_blend_finished(m_animation_index_attack))
		{
			if (m_obj.get_animation_cycle_count() >= m_animation_attack_repeat_count)
			{
				on_attack_complete();
			}
		}
	}
	else if (m_current_state == e_cs_win)
	{
		if (m_obj.is_blend_finished(m_animation_index_win))
		{
			if (m_obj.get_animation_cycle_count() >= 1)
			{
				on_win_complete();
			}
		}
	}
	else if (m_current_state == e_cs_death)
	{
		if (m_death_type == e_kill_type::e_kill_by_enemy)
		{
			if (m_obj.is_blend_finished(m_animation_index_death))
			{
				if (m_obj.get_animation_cycle_count() >= 1)
				{
					on_death_complete();
				}
			}
		}
		else if (m_death_type == e_kill_type::e_kill_by_fire)
		{
			if (m_fire_death_state == e_death_state_wait)
			{
				if (std::chrono::system_clock::now() >= m_fire_death_wait_timeout)
				{
					m_fire_death_state = e_death_state_ashfall_waiting_for_clipping_start;
				}
			}
			else if (m_fire_death_state == e_death_state_ashfall_waiting_for_clipping_start)
			{
				// ashfall started when clipped points > 0
				if (m_clipped_points_count > 0)
				{
					m_fire_death_state = e_death_state_ashfall_started;
				}
			}
			else if (m_fire_death_state == e_death_state_ashfall_started)
			{
				if (m_obj.get_ashfalls_count() == 0)	// wait until all ashfalls finished
				{
					m_fire_death_state = e_death_state_ashfall_finished;

					on_death_complete();
				}
			}
		}
	}
}


void character3d_c::update_ashfalls(camera_t* cam_ptr)
{
	m_obj.update_ashfalls(cam_ptr);

	if (m_fire_death_state == e_death_state_ashfall_waiting_for_clipping_start ||
		m_fire_death_state == e_death_state_ashfall_started ||
		m_fire_death_state == e_death_state_ashfall_finished)
	{
		m_y_clip_plane += g_last_frame_duration * 0.05f;

		m_clipped_points_count = m_obj.clip_by_y_plane(m_obj.get_render_buffer(),
			m_y_clip_plane);
	}
}


void character3d_c::fill_renderbuffer()
{
	m_obj.fill_renderbuffer();
}


void character3d_c::ashfall_fill_renderbuffer(sr::render_buffer_t& render_buffer)
{
	m_obj.ashfall_fill_renderbuffer(render_buffer);
}


render_buffer_t* character3d_c::get_render_buffer()
{
	return m_obj.get_render_buffer();
}


void character3d_c::on_start_movement()
{
	m_obj.set_animation_repeats(0);
	m_obj.blend_animation(m_animation_index_run, m_animation_blend_time);
}


void character3d_c::on_stop_movement()
{
	m_obj.set_animation_repeats(0);
	m_obj.blend_animation(m_animation_index_idle, m_animation_blend_time);
}


void character3d_c::attack()
{
	character_ai_c<game_map3d_c>::attack();

	m_obj.set_animation_repeats(m_animation_attack_repeat_count);
	m_obj.blend_animation(m_animation_index_attack, m_animation_blend_time);
}


void character3d_c::enter_win_state()
{
	character_base_c::enter_win_state();

	m_obj.set_animation_repeats(1);
	m_obj.blend_animation(m_animation_index_win, m_animation_blend_time);
}


void character3d_c::kill(e_kill_type kill_type)
{
	character_ai_c<game_map3d_c>::kill(kill_type);

	if (kill_type == e_kill_type::e_kill_by_enemy)
	{
		m_obj.set_animation_repeats(1);
		m_obj.blend_animation(m_animation_index_death, m_animation_blend_time);
	}
	else if (kill_type == e_kill_type::e_kill_by_fire)
	{
		// todo: stop movement

		m_obj.pause_animation();

		m_obj.set_material_library(&m_materials[e_material_set_burnt]);

		m_fire_death_state = e_death_state_wait;
		m_fire_death_wait_timeout = std::chrono::system_clock::now()
			+ std::chrono::milliseconds(700);
	}
}
