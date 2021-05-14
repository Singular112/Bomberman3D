#include "stdafx.h"
#include "enemy3d_c.h"

enemy3d_c::enemy3d_c()
{
	set_speed(1);
	set_state(e_cs_idle);
}


void enemy3d_c::update()
{
	character3d_c::update();

	//
	m_obj.set_y_angle((float)m_angle);

	m_obj.update(false);

	//m_obj.cull_backfaces(&cam);

	m_obj.update_vertex_normals();

	m_obj.compute_light(get_light_source(0),
		get_light_sources_count());

	auto global_pos = m_game_map_ptr->translate_local_pos_to_map_global_pos
	(
		glm::vec3(m_x_center_pos, 0, m_y_center_pos),
		false
	);

	m_obj.set_pos(global_pos);
}

// ===============================================

void flower_enemy3d_c::create()
{
	auto mesh = mesh_factory_c::me()->get_mesh(mesh_factory_c::e_mesh_flower);
	if (!mesh)
	{
		return;
	}

	m_obj.set_scale(1.0f, 1.0f, 1.0f);

	m_obj.assign_mesh(mesh);

	character3d_c::create();

	m_animation_index_run = m_obj.get_animation_index_by_name("Armature|run");
	m_animation_index_idle = m_obj.get_animation_index_by_name("Armature|idle");

	m_obj.set_animation_rate(1.0f);

	m_obj.set_animation(m_animation_index_idle);

	m_obj.start_animation();
}


void devil_enemy3d_c::create()
{
	auto mesh = mesh_factory_c::me()->get_mesh(mesh_factory_c::e_mesh_devil);
	if (!mesh)
	{
		return;
	}

	m_obj.set_scale(0.8f, 0.8f, 0.8f);

	m_obj.assign_mesh(mesh);

	enemy3d_c::create();

	m_animation_index_run = m_obj.get_animation_index_by_name("Armature|run");
	m_animation_index_idle = m_obj.get_animation_index_by_name("Armature|idle");

	m_obj.set_animation_rate(1.0f);

	m_obj.set_animation(m_animation_index_idle);

	m_obj.start_animation();
}


void frog_enemy3d_c::create()
{
	auto mesh = mesh_factory_c::me()->get_mesh(mesh_factory_c::e_mesh_frog);
	if (!mesh)
	{
		return;
	}

	m_obj.set_scale(0.8f, 0.8f, 0.8f);

	m_obj.assign_mesh(mesh);

	enemy3d_c::create();

	m_obj.set_scale(0.8f, 0.8f, 0.8f);

	m_animation_index_run = m_obj.get_animation_index_by_name("Armature|run");
	m_animation_index_idle = m_obj.get_animation_index_by_name("Armature|idle");

	m_obj.set_animation_rate(1.0f);

	m_obj.set_animation(m_animation_index_idle);

	m_obj.start_animation();
}
