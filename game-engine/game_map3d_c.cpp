#include "stdafx.h"
#include "game_map3d_c.h"

#include "game_types.h"
#include "mesh_factory_c.h"
#include "object_bomb3d_c.h"
#include "modifier_object3d_c.h"
#include "object_exit3d_c.h"

using namespace sr::texture;


game_map3d_c::game_map3d_c()
{
}


game_map3d_c::~game_map3d_c()
{
}


void game_map3d_c::update()
{
	m_plane_bottom_obj.update();

	for (auto& cell : m_cells)
	{
		auto cell_type = cell.get_cell_type();

		if (cell_type != e_gt_free_cell)
		{
			if (cell_type == e_gt_exit)
			{
				cell.set_cell_type(e_gt_exit);
			}

			auto obj3d = cell.get_current_game_object();
			obj3d->update();

			if (cell_type == e_game_obj_type::e_gt_bomb && obj3d->is_destroyed())
			{
				for (auto& boom_callback : m_on_boom_callbacks)
				{
					boom_callback(&cell);
				}
			}
		}
	}
}


void game_map3d_c::fill_renderbuffer(render_buffer_t& render_buffer)
{
	m_plane_bottom_obj.fill_renderbuffer(render_buffer);

	for (auto& cell : m_cells)
	{
		auto cell_type = cell.get_cell_type();

		if (cell_type == e_gt_exit)
		{
			cell.set_cell_type(e_gt_exit);
		}

		if (cell_type != e_gt_free_cell)
		{
			auto obj3d = cell.get_current_game_object();
			obj3d->fill_renderbuffer(render_buffer);
		}
	}
}


bool game_map3d_c::initialize_resources()
{
	auto wall_tex = resource_manager_c::me()->get_texture
	(
		resource_manager_c::e_texture_wall
	);

	auto box_tex = resource_manager_c::me()->get_texture
	(
		resource_manager_c::e_texture_box
	);

	auto floor_tex = resource_manager_c::me()->get_texture
	(
		resource_manager_c::e_texture_floor
	);

	// wall material
	{
		auto& material = m_material_library[e_mesh_type::e_mt_wall];

		material.shade_model = sr::e_sm_shade_flat;
		material.is_wireframe = false;
		material.texture2d = wall_tex;
		material.texture_mode = sr::e_tm_affine;
		material.attr = material.shade_model | material.texture_mode;
		material.k_ambient =
			material.k_diffuse =
			material.k_specular = 1.0f;
		material.k_ambient_color = { 255, 255, 255, 255 };
		material.k_diffuse_color = { 255, 255, 255, 255 };
		material.k_specular_color.color = XRGB(0, 0, 0);
	}

	// floor material
	{
		auto& material = m_material_library[e_mesh_type::e_mt_floor];

		material.shade_model = sr::e_sm_shade_flat;
		material.is_wireframe = false;
		material.texture2d = floor_tex;
		material.texture_mode = sr::e_tm_affine;
		material.attr = material.shade_model | material.texture_mode;
		material.k_ambient =
			material.k_diffuse =
			material.k_specular = 1.0f;
		material.k_ambient_color = { 255, 255, 255, 255 };
		material.k_diffuse_color = { 255, 255, 255, 255 };
		material.k_specular_color.color = XRGB(0, 0, 0);
	}

	// box material
	{
		auto& material = m_material_library[e_mesh_type::e_mt_box];

		material.shade_model = sr::e_sm_shade_flat;
		material.is_wireframe = false;
		material.texture2d = box_tex;
		material.texture_mode = sr::e_tm_affine;
		material.attr = material.shade_model | material.texture_mode;
		material.k_ambient =
			material.k_diffuse =
			material.k_specular = 1.0f;
		material.k_ambient_color = { 255, 255, 255, 255 };
		material.k_diffuse_color = { 255, 255, 255, 255 };
		material.k_specular_color.color = XRGB(0, 0, 0);
	}

	return true;
}


void game_map3d_c::initialize_cell3d(map_cell3d_c& cell)
{
	glm::vec3 cell_local_position
	(
		cell.get_cell_x_index() * m_cell_width,
		0.0f,
		cell.get_cell_y_index() * m_cell_width
	);

	glm::vec3 cell_global_position
	(
		cell_local_position.x + m_absolute_position.x,
		cell_local_position.y + m_absolute_position.y,
		cell_local_position.z + m_absolute_position.z
	);

	cell.set_local_position(glm::vec4(cell_local_position, 1.0f));
	cell.set_global_position(glm::vec4(cell_global_position, 1.0f));

	auto get_cell_mesh_type = [this](map_cell3d_c& cell) -> e_mesh_type
	{
		auto typ = cell.get_cell_type();

		switch (typ)
		{
		case e_gt_wall:
			return e_mesh_type::e_mt_wall;
		case e_gt_box:
			return e_mesh_type::e_mt_box;
		}

		return e_mesh_type::e_mt_none;
	};

	if (cell.get_cell_type() == e_gt_free_cell)
	{
		return;
	}

	auto obj3d = new game_object_c(false);
	{
		obj3d->set_game_type(cell.get_cell_type());

		obj3d->assign_mesh(m_mesh_library[get_cell_mesh_type(cell)].get());

		obj3d->set_scale((float)m_cell_width, (float)m_cell_height, (float)m_cell_width);

		obj3d->set_polygons_attributes(POLY_ATTR_ZBUFFER);

		obj3d->set_pos(cell_global_position);
	}
	cell.append_game_object(obj3d);
}


void game_map3d_c::create(int x_cell_count, int y_cell_count,
	int cell_width, int cell_height)
{
	m_cell_x_count = x_cell_count;
	m_cell_y_count = y_cell_count;

	m_cell_width = cell_width;
	m_cell_height = cell_height;

	m_box_cells_count = 0;

	m_width = (float)(m_cell_x_count * m_cell_width);
	m_height = (float)(y_cell_count * m_cell_width);

	if (!initialize_resources())
	{
		return;
	}

	m_cells.resize(m_cell_x_count * m_cell_y_count);

	if (m_absolute_position_mode == e_mpm_by_center)
	{
		float half_width = m_width * 0.5f;
		float half_height = m_height * 0.5f;

		m_absolute_position.x -= half_width;
		m_absolute_position.z -= half_height;
	}

	// create floor
	auto floor_ptr = mesh_base_ptr_t(mesh_factory_c::generate_plane
	(
		m_width,
		m_height,
		false,
		&m_material_library[e_mesh_type::e_mt_floor]
	));
	m_mesh_library[e_mesh_type::e_mt_floor] = std::move(floor_ptr);
	m_plane_bottom_obj.assign_mesh(m_mesh_library[e_mesh_type::e_mt_floor].get());
	m_plane_bottom_obj.set_polygons_attributes(POLY_ATTR_ZBUFFER);
	m_plane_bottom_obj.set_pos(m_absolute_position);

	// create wall mesh
	auto wall_ptr = mesh_base_ptr_t(mesh_factory_c::generate_mesh<mesh_factory_c::cube_obj>
	(
		1.0f,
		false,
		&m_material_library[e_mesh_type::e_mt_wall]
	));
	m_mesh_library[e_mesh_type::e_mt_wall] = std::move(wall_ptr);

	// create box mesh
	auto box_ptr = mesh_base_ptr_t(mesh_factory_c::generate_mesh<mesh_factory_c::cube_obj>
	(
		1.0f,
		false,
		&m_material_library[e_mesh_type::e_mt_box]
	));
	m_mesh_library[e_mesh_type::e_mt_box] = std::move(box_ptr);

	for (int y = 0; y < m_cell_y_count; ++y)
	{
		int line_shift = y * m_cell_x_count;

		for (int x = 0; x < m_cell_x_count; ++x)
		{
			int cell_index = line_shift + x;
			auto& cell = m_cells[cell_index];

			cell.set_cell_size(m_cell_width);
			cell.set_cell_index(cell_index);
			cell.set_cell_indexes(x, y);

			if (y % 2 == 1 && x % 2 == 1)
			{
				cell.set_cell_type(e_gt_wall);
			}
#if 0
			else
			{
				if (special_cells.find(cell_index) != special_cells.cend())
				{

				}

				else if (rand() % 100 > 80)
				{
					cell.set_cell_type(e_gt_box);

					m_box_cells_count++;
				}
				else
				{
					cell.set_cell_type(e_gt_free_cell);
				}
			}
#endif

			initialize_cell3d(cell);
		}
	}

	std::vector<cell_index_s> busy_cells =
	{
		{ 5, 0 },
		{ 8, 0 },
		{ 6, 2 }
	};

	std::vector<cell_index_s> free_cells =
	{
		{ 6, 0 },
		{ 6, 1 },
		{ 7, 0 },
	};

	for (const auto& cell_indices : busy_cells)
	{
		int cell_index = cell_indices.y * m_cell_x_count + cell_indices.x;

		auto& cell = m_cells[cell_index];
		cell.set_cell_type(e_gt_box);
		initialize_cell3d(cell);

		m_box_cells_count++;
	}

#if 1
	int cell_spawn_count = 30;
	while (cell_spawn_count > 0)
	{
		int x = rand() % m_cell_x_count;
		int y = rand() % m_cell_y_count;

		bool skip = false;
		for (auto free_cell : free_cells)
		{
			if (free_cell.x == x && free_cell.y == y)
			{
				skip = true;
				break;
			}
		}

		if (skip)
		{
			continue;
		}

		auto cell = get_cell(x, y);
		if (cell && cell->get_cell_type() == e_game_obj_type::e_gt_free_cell)
		{
			cell->set_cell_type(e_gt_box);
			initialize_cell3d(*cell);

			m_box_cells_count++;

			cell_spawn_count--;
		}
	}
#else
	int cell_spawn_count = 30;
	while (cell_spawn_count > 0)
	{
		int x = rand() % m_cell_x_count;
		int y = rand() % m_cell_y_count;

		auto cell = get_cell(x, y);
		if (cell && cell->get_cell_type() == e_game_obj_type::e_gt_free_cell)
		{
			place_game_object(cell, e_game_obj_type::e_gt_exit);

			cell_spawn_count--;
		}
	}
#endif
}


void game_map3d_c::set_absolute_position(const glm::vec4 pos,
	e_map_position_mode position_mode)
{
	m_absolute_position = pos;
	m_absolute_position_mode = position_mode;
}


glm::vec3 game_map3d_c::get_cell_position(int cell_x_index,
	int cell_y_index,
	bool get_center_position) const
{
	glm::vec3 result;

	if (get_center_position)
	{
		result.x =
			(cell_x_index * m_cell_width) +
			(m_cell_width * 0.5f);

		result.y = 0.0f;

		result.z =
			(cell_y_index * m_cell_width) +
			(m_cell_width * 0.5f);
	}
	else
	{
		result.x =
			(float)(cell_x_index * m_cell_width);

		result.y = 0.0f;

		result.z =
			(float)(cell_y_index * m_cell_width);
	}

	return result;
}


glm::vec3 game_map3d_c::translate_local_pos_to_map_global_pos(const glm::vec3& local_pos,
	bool center) const
{
	glm::vec3 result;

	float shift_value = center ? (m_cell_width * 0.5f) : 0.0f;

	result.x = m_absolute_position.x + shift_value + local_pos.x;
	result.y = m_absolute_position.y + local_pos.y;

#if 0
	float z_inv = (float)(m_cell_y_count * m_cell_width);

	z_inv = z_inv - local_pos.z;

	result.z = m_absolute_position.z - shift_value + z_inv;
#else
	result.z = m_absolute_position.z + shift_value + local_pos.z;
#endif

	return result;
}


void game_map3d_c::place_game_object(map_cell3d_c* cell,
	e_game_obj_type type,
	e_game_modifier modifier_type)
{
	game_object_c* object = nullptr;

	if (type == e_game_obj_type::e_gt_bomb)
	{
		object = new object_bomb3d_c(3000);
	}
	else if (type == e_game_obj_type::e_gt_modifyer)
	{
		if (modifier_type == e_modifier_unknown)
			return;

		object = new modifier_object3d_c(modifier_type);
	}
	else if (type == e_game_obj_type::e_gt_exit)
	{
		object = new object_exit3d_c();

		auto global_pos = translate_local_pos_to_map_global_pos
		(
			cell->get_local_position(),
			false
		);

		object->set_pos(global_pos);

		cell->append_game_object(object);

		return;
	}
	else
	{
		return;
	}

	assert(object != nullptr);

	auto global_pos = translate_local_pos_to_map_global_pos
	(
		cell->get_local_position(),
		true
	);

	object->set_pos(global_pos);

	if (object->is_animated())
	{
		object->start_animation();
	}

	cell->append_game_object(object);
}
