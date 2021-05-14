#include "stdafx.h"
#include "bomberman3d_c.h"
#include "object_bomb3d_c.h"
#include "modifier_object3d_c.h"

void bomberman3d_c::create()
{
	auto mesh = mesh_factory_c::me()->get_mesh(mesh_factory_c::e_mesh_bomberman);
	if (!mesh)
	{
		return;
	}

	m_obj.set_scale(0.8f, 0.8f, 0.8f);

	m_obj.assign_mesh(mesh);

	character3d_c::create();

	m_animation_index_run = m_obj.get_animation_index_by_name("Armature|run");
	m_animation_index_win = m_obj.get_animation_index_by_name("Armature|floating");
	m_animation_index_idle = m_obj.get_animation_index_by_name("Armature|idle");
	m_animation_index_attack = m_obj.get_animation_index_by_name("Armature|attack");
	m_animation_index_death = m_obj.get_animation_index_by_name("Armature|death");

	m_obj.set_animation_rate(1.0f);

	m_obj.set_animation(m_animation_index_idle);

	m_obj.start_animation();
}


void bomberman3d_c::update()
{
	character3d_c::update();
	update_contol();

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


void bomberman3d_c::set_direction(e_way_direction new_direction)
{
	character_base_c::set_direction(new_direction);
}


void bomberman3d_c::set_current_input_state(e_input_states state)
{
	character3d_c::set_current_input_state(state);

	if (m_current_input_state == state_none)
	{
		on_stop_movement();
	}
}


void bomberman3d_c::place_game_object(e_game_obj_type type)
{
	auto current_cell = dynamic_cast<map_cell3d_c*>(get_current_cell());
	if (!current_cell)
		return;

	if (type == e_game_obj_type::e_gt_bomb)
	{
		auto current_cell_type = current_cell->get_cell_type();

		if (current_cell_type == e_game_obj_type::e_gt_exit ||
			current_cell_type == e_game_obj_type::e_gt_bomb)
		{
			return;
		}

		if (m_bombs_already_on_map >= m_current_modifiers_setup.bombs_amount)
		{
			return;
		}

		m_bombs_already_on_map++;
	}

	m_game_map_ptr->place_game_object(current_cell, type);
}


int bomberman3d_c::get_explosion_power() const
{
	return m_current_modifiers_setup.explosion_power;
}


void bomberman3d_c::on_bomb_explosion(map_cell3d_c* bomb_cell)
{
	m_bombs_already_on_map--;
}
